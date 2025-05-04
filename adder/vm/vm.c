#include "vm.h"
#include "sh_types.h"
#include "sh_asminfo.h"
#include "sh_value.h"
#include "sh_utils.h"
#include "sh_config.h"
#include "sh_program.h"
#include "vm_env.h"
#include "vm_heap.h"
#include "vm_validate.h"
#include <sh_log.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <dlfcn.h>
#include <unistd.h>

#if VM_TRACE_LOG_LEVEL > 0

# define TRACE_LOG(...) sh_log_info(__VA_ARGS__)
# define TRACE_OP(C) TRACE_LOG("> %s ", get_op_name(C))
# define TRACE_INT_ARG(A) TRACE_LOG("%i ", (A))
# define TRACE_NL() sh_log_info("\n");

# if VM_TRACE_LOG_LEVEL > 1
void sprint_stack(val_t* stack, int stack_size) {
    sh_log_info(" stack (s:%i) | ", stack_size);
    define_cstr(str, 32);
    for(int i = 0; i < stack_size; i++) {
        val_sprint(str, stack[i]);
        cstr_append_fmt(str, " ");
    }
    sh_log_info("%s\n");
}
#  define TRACE_PRINT_STACK(S, TOP) sprint_stack((S), (TOP) + 1)
# else
#  define TRACE_PRINT_STACK(S, TOP)
# endif

#else
# define TRACE_LOG(...)
# define TRACE_OP(C)
# define TRACE_INT_ARG(A)
# define TRACE_PRINT_STACK(S, TOP)
# define TRACE_NL()
#endif

val_t* gvm_addr_lookup(void* user, val_addr_t addr) {
    vm_t* VM = (vm_t*) user;
    int offset = MEM_ADDR_TO_INDEX(addr);
    if( ADDR_IS_CONST(addr) ) {
        return VM->run.constants + offset;
    } else {
        return VM->mem.membase + offset;
    }
}

void vm_sprint_val(cstr_t str, vm_t* vm, val_t val) {
    val_sprint_lookup(str, val, &gvm_addr_lookup, vm);
}

int vm_get_string(vm_t* vm, val_t val, char* dest, int dest_len) {
    return val_get_string(val, &gvm_addr_lookup, vm, dest, dest_len);
}

bool vm_create(vm_t* vm, int memory_size) {

    if( memory_size > MEM_MAX_ADDRESSABLE ) {
        sh_log_warning("warning: the requested VM memory size %i is too large.\n"
                "\tmaximum addressable memory is %i.\n",
                memory_size,
                MEM_MAX_ADDRESSABLE);
        return false;
    }

    int dyn_size = memory_size / 2;
    int stack_size = memory_size / 2;

    int memsize = memory_size * sizeof(val_t);
    val_t* mem = (val_t*) malloc( memsize );
    if( mem == NULL ) {
        sh_log_error("could'nt allocate VM memory.\n");
        return false;
    }

    // size: one bit per val_t
    uint64_t* gc_marks = (uint64_t*) malloc(CALC_GC_MARK_U64_COUNT(dyn_size) * sizeof(uint64_t));
    if( gc_marks == NULL ) {
        sh_log_error("could'nt allocate GC mark region.\n");
        free(mem);
        return false;
    }
    memset(gc_marks, 0, CALC_GC_MARK_U64_COUNT(dyn_size) * sizeof(uint64_t));
    
    vm->mem.membase = mem;
    vm->mem.memsize = memory_size;
    
    vm->mem.stack.values = vm->mem.membase;
    vm->mem.stack.size = stack_size;
    vm->mem.stack.top = -1;

    // heap & GC
    vm->mem.heap.values = vm->mem.membase + stack_size;
    vm->mem.heap.size = dyn_size;
    vm->mem.heap.gc_marks = gc_marks;

    // assigend on execution
    vm->run = (vm_runtime_t) { 0 };

    VALIDATION_INIT(vm);

    return true;
}

void vm_destroy(vm_t* vm) {
    if( vm == NULL || vm->mem.membase == 0 ) {
        return;
    }
        
    VALIDATION_DESTROY(vm);
    free(vm->mem.membase);
    free(vm->mem.heap.gc_marks);
    memset(vm, 0, sizeof(vm_t));
}

inline static void ffi_invoke(ffi_handle_t* hndl, int arg_count, vm_t* vm) {
    switch(hndl->tag) {
        case FFI_HNDL_HOST_ACTION: {
            hndl->u.host_action(
                (ffi_hndl_meta_t) {
                    .local = hndl->local,
                    .vm = vm
                },
                arg_count,
                vm->mem.stack.values + vm->mem.stack.top + 1 - arg_count);
            vm->mem.stack.top -= arg_count;
        } break;
        case FFI_HNDL_HOST_FUNCTION: {
            val_t ret = hndl->u.host_function(
                (ffi_hndl_meta_t) {
                    .local = hndl->local,
                    .vm = vm
                },
                arg_count,
                vm->mem.stack.values + vm->mem.stack.top + 1 - arg_count);
            vm->mem.stack.top -= arg_count;
            vm->mem.stack.values[++vm->mem.stack.top] = ret;
        } break;
        default: {
            assert(false && "Not implemented");
        }
    }
}

void vm_select_entry_point(vm_t* vm, program_t* program, uint32_t address) {
    assert( program->inst.size >= address );
    if( program->inst.buffer[address] == OP_MAKE_FRAME ) {
        // push negative number as return address
        vm->mem.stack.values[++vm->mem.stack.top] = val_number(-1.0f);
    }
    // jump to label / function
    vm->run.pc = address;
}

val_t vm_execute(vm_t* vm, vm_env_t* env, entry_point_t* ep, program_t* program) {

    assert(sizeof(float) == 4);

    assert(program != NULL);

    if( vm_env_is_ready(env) == false ) {
        sh_log_error("incomplete vm env, cannot start execution");
        return val_number(-1099);
    }
    
    val_t* stack = vm->mem.stack.values;
    val_t* consts = program->cons.buffer;
    uint8_t* instructions = program->inst.buffer;

    vm_runtime_t* vm_run = &vm->run;
    vm_run->constants = consts;
    vm_run->instructions = instructions;
    vm_run->pc = 0;

    vm_mem_t* vm_mem = &vm->mem;
    memset(vm_mem->stack.values, 0, sizeof(val_t) * vm_mem->stack.size);

    // push initial args (if any)
    vm_mem->stack.frame = -1;
    vm_mem->stack.top = -1;

    for(int i = 0; i < ep->argcount; i++) {
        stack[++vm_mem->stack.top] = ep->argvals[i];
    }
    
    uint32_t cycles_remaining = (uint32_t) 1000000;
    if (program->inst.size == 0) {
        cycles_remaining = 0;
    }

    assert(ep->address >= 0);
    vm_select_entry_point(vm, program, ep->address);

    while ( (cycles_remaining--) != 0 ) {

        vm_op_t opcode = instructions[vm_run->pc++];

        TRACE_OP(opcode);

        VALIDATE_PRE(vm, opcode);

        assert(OP_OPCODE_COUNT == 37 && "Opcode count changed.");

        switch (opcode) {
            case OP_PUSH_VALUE: {
                uint32_t const_index = READ_U32(instructions, vm_run->pc);
                TRACE_INT_ARG(const_index);
                stack[++vm_mem->stack.top] = consts[const_index];
                vm_run->pc += 4;
            } break;
            case OP_POP_1: {
                vm_mem->stack.top -= 1;
            } break;
            case OP_POP_2: {
                vm_mem->stack.top -= 2;
            } break;
            case OP_ADD: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_number(a + b);
            } break;
            case OP_SUB: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_number(a - b);
            } break;
            case OP_MUL: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_number(a * b);
            } break;
            case OP_DIV: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_number(a / b);
            } break;
            case OP_MOD: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_number((int) a % (int) b);
            } break;
            case OP_NEG: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_number(-a);
            } break;
            case OP_CMP_LESS_THAN: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a < b );
            } break;
            case OP_CMP_LESS_THAN_OR_EQUAL: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a <= b );
            } break;
            case OP_CMP_MORE_THAN: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a > b );
            } break;
            case OP_CMP_MORE_THAN_OR_EQUAL: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a >= b );
            } break;
            case OP_CMP_EQUAL: {
                // todo: other types than numbers
                const float epsilon = 0.0001f;
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( fabs(a - b) < epsilon );
            } break;
            case OP_CMP_NOT_EQUAL: {
                // todo: other types than numbers
                const float epsilon = 0.0001f;
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( fabs(a - b) > epsilon );
            } break;
            case OP_AND: {
                bool a = val_into_bool(stack[vm_mem->stack.top--]);
                bool b = val_into_bool(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a && b );
            } break;
            case OP_OR: {
                bool a = val_into_bool(stack[vm_mem->stack.top--]);
                bool b = val_into_bool(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a || b );
            } break;
            case OP_NOT: {
                bool a = val_into_bool(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( !a );
            } break;
            case OP_DUP_1: {
                val_t a = stack[vm_mem->stack.top];
                stack[++vm_mem->stack.top] = a;
            } break;
            case OP_DUP_2: {
                val_t a = stack[vm_mem->stack.top - 1];
                val_t b = stack[vm_mem->stack.top];
                stack[++vm_mem->stack.top] = a;
                stack[++vm_mem->stack.top] = b;
            } break;
            case OP_ROT_2: {
                val_t a = stack[vm_mem->stack.top - 1];
                val_t b = stack[vm_mem->stack.top];
                stack[vm_mem->stack.top - 1] = b;
                stack[vm_mem->stack.top]     = a;
            } break;
            case OP_JUMP: {
                vm_run->pc = READ_U32(instructions, vm_run->pc);
                TRACE_INT_ARG(vm_run->pc);
            } break;
            case OP_JUMP_IF_FALSE: {
                TRACE_INT_ARG(READ_U32(instructions, vm_run->pc));
                if( val_into_bool(stack[vm_mem->stack.top--]) == false ) {
                    vm_run->pc = READ_U32(instructions, vm_run->pc);
                } else {
                    vm_run->pc += 4;
                }
            } break;
            case OP_HALT:{
                TRACE_NL();
                return val_number(-1002);
            } break;
            case OP_EXIT: {
                TRACE_NL();
                uint32_t return_value = READ_U32(instructions, vm_run->pc);
                TRACE_INT_ARG(return_value);
                return val_number(return_value);
            } break;
            case OP_CALL: {
                // push the return address
                stack[++vm_mem->stack.top] = val_number(vm_run->pc + 4);
                // jump to label / function
                vm_run->pc = READ_U32(instructions, vm_run->pc);
                TRACE_INT_ARG(vm_run->pc);
            } break;
            case OP_MAKE_FRAME: {

                uint32_t nargs = READ_U32(instructions, vm_run->pc);
                TRACE_INT_ARG(nargs);
                vm_run->pc += 4;

                uint32_t nlocals = READ_U32(instructions, vm_run->pc);
                TRACE_INT_ARG(nlocals);
                vm_run->pc += 4;

                // stack top should now be the return address
                // Note: if the return address is negative we exit the vm
                //       returning the top of stack element.
                float call_site = val_into_number(stack[vm_mem->stack.top--]);
                frame_t frame = (frame_t) {
                    .return_pc = (int) call_site,
                    .num_args = nargs,
                    .num_locals = nlocals
                };

                // move args to after the frame
                int frame_start = vm_mem->stack.top - nargs + 1;
                for(int i = nargs; i > 0; i--) {
                    // stack[i+N] = stack[i+N-1]
                    stack[frame_start + i] = stack[frame_start + i - 1];
                }

                // insert frame before locals
                stack[frame_start] = val_frame(frame);
                vm_mem->stack.frame = frame_start;

                // OBS: ZERO INIT MIGHT NOT BE NEEDED!!
                // init locals (not needed)
                uint32_t locals_idx = frame_start + 1 + nargs;
                for(uint32_t i = 0; i < nlocals; i++) {
                    stack[locals_idx + i] = (val_t) { 0 };
                }

                // make room for the locals on the stack
                vm_mem->stack.top += nlocals + 1;

            } break;
            case OP_RETURN_NOTHING: {

                frame_t frame;
                if( vm_mem->stack.frame >= 0 ) {
                    frame = val_into_frame(stack[vm_mem->stack.frame]);
                } else {
                    frame.return_pc = -1;
                }

                // Note: if the return address is negative we exit the vm.
                if( frame.return_pc < 0 ) {
                    // below the return value in the stack
                    // we may have a frame + locals etc
                    // so to ensure the frame is poped
                    // we reset the stack top pointer
                    vm_mem->stack.top = 0;
                    return val_none();
                }

                // update pc to resume at call site
                vm->run.pc = frame.return_pc;

                // hop down to parent call frame - 1
                vm_mem->stack.top = vm_mem->stack.frame - 1;

                // find index of previous frame
                // this way of updating current frame may turn out
                // to be too slow.
                vm_mem->stack.frame = -1;
                for(int i = vm_mem->stack.top; i >= 0; i--) {
                    if( stack[i].type == VAL_FRAME ) {
                        vm_mem->stack.frame = i;
                        break;
                    }
                }
            } break;
            case OP_RETURN_VALUE: {

                frame_t frame;
                if( vm_mem->stack.frame >= 0 ) {
                    frame = val_into_frame(stack[vm_mem->stack.frame]);
                } else {
                    frame.return_pc = -1;
                }

                if( frame.return_pc < 0 ) {
                    // Note: if the return address is negative we exit the vm
                    //       returning the top of stack element. 
                    val_t rval = val_none();
                    if( vm_mem->stack.top >= 0 ) {
                        rval = stack[vm_mem->stack.top];
                        // below the return value in the stack
                        // we may have a frame + locals etc
                        // so to ensure the frame is poped
                        // we reset the stack top pointer
                        vm_mem->stack.top = 0;
                    }

                    return rval;
                }

                int body_start = vm_mem->stack.frame + frame.num_locals + frame.num_args;
                int invoked_top = vm_mem->stack.top;
                // copy possible return value
                val_t ret_val = stack[invoked_top];
                // hop down to parent call frame - 1
                vm_mem->stack.top = vm_mem->stack.frame - 1;
                // update pc to resume at call site
                vm->run.pc = frame.return_pc; 
                // check if we have a return value
                if(invoked_top > body_start) {
                    // push return value
                    stack[++vm_mem->stack.top] = ret_val;
                }

                // find index of previous frame
                // this way of updating current frame may turn out
                // to be too slow.
                vm_mem->stack.frame = -1;
                for(int i = vm_mem->stack.top; i >= 0; i--) {
                    if( stack[i].type == VAL_FRAME ) {
                        vm_mem->stack.frame = i;
                        break;
                    }
                }

            } break;
            case OP_STORE_LOCAL: {
                uint32_t local_idx = READ_U32(instructions, vm_run->pc);
                TRACE_INT_ARG(local_idx);
                stack[vm_mem->stack.frame + 1 + local_idx] = stack[vm_mem->stack.top--];
                vm_run->pc += 4;
            } break;
            case OP_LOAD_LOCAL: {
                uint32_t local_idx = READ_U32(instructions, vm_run->pc);
                TRACE_INT_ARG(local_idx);
                stack[++vm_mem->stack.top] = stack[vm_mem->stack.frame + 1 + local_idx];
                vm_run->pc += 4;
            } break;
            case OP_MAKE_ARRAY: {
                // pop array size
                val_t size = stack[vm_mem->stack.top--];
                uint32_t count = val_into_number(size);
                // allocate array
                array_t array = heap_array_alloc(vm, count);
                if( ADDR_IS_NULL(array.address) ) {
                    sh_log_error("\nheap alloc failed\n");
                    return val_number(-1005);
                }
                // copy all data to the array
                val_t* source_ptr = &stack[vm_mem->stack.top + 1 - count];
                heap_array_copy_to(vm, source_ptr, count, array);
                // remove the data from the stack
                vm_mem->stack.top -= count; 
                stack[++vm_mem->stack.top] = val_array(array);
            } break;
            case OP_ARRAY_LENGTH: {
                val_t array_val = stack[vm_mem->stack.top--];
                array_t array = val_into_array(array_val);
                stack[++vm_mem->stack.top] = val_number(array.length);
            } break;
            case OP_MAKE_ITER: {
                val_t array_val = stack[vm_mem->stack.top--];
                array_t array = val_into_array(array_val);
                iter_t iter = (iter_t) {
                    .current = array.address,
                    .remaining = array.length
                };
                stack[++vm_mem->stack.top] = val_iter(iter);
            } break;
            case OP_ITER_NEXT: {
                uint32_t exit_pc = READ_U32(instructions, vm_run->pc);
                TRACE_INT_ARG(exit_pc);
                val_t iter_val = stack[vm_mem->stack.top];
                iter_t iter = val_into_iter(iter_val);
                if( iter.remaining == 0 ) {
                    vm_mem->stack.top --;
                    vm_run->pc = exit_pc;
                } else {
                    uint32_t mem_index = MEM_ADDR_TO_INDEX(iter.current);
                    val_t value = vm_mem->membase[mem_index];
                    iter.remaining -= 1;
                    iter.current = MEM_MK_PROGR_ADDR(mem_index + 1);
                    stack[vm_mem->stack.top] = val_iter(iter);
                    stack[++vm_mem->stack.top] = value;
                    vm_run->pc += 4;
                }
            } break;
            case OP_CALL_NATIVE: {
                uint32_t findex = READ_U32(instructions, vm_run->pc);
                TRACE_INT_ARG(findex);
                ffi_handle_t* handle = &env->handles[findex];
                int arg_count = env->argcounts[findex];
                ffi_invoke(handle, arg_count, vm);
                vm_run->pc += 4;
            } break;
            default: {
                char* op_str = get_op_name(opcode);
                sh_log_error("\nunhandled operatioin %i (%s)\n", opcode, op_str);
                return val_number(-1003);
            } break;
        }

        TRACE_NL();
        VALIDATE_POST(vm, opcode);
        TRACE_PRINT_STACK(vm_mem->stack.values, vm_mem->stack.top);
    }

    return val_number(-1004);
}

