#include "gvm.h"
#include "gvm_parser.h"
#include "gvm_asm.h"
#include "gvm_env.h"
#include "gvm_types.h"
#include "gvm_value.h"
#include "gvm_utils.h"
#include "gvm_config.h"
#include "gvm_memory.h"
#include "gvm_heap.h"
#include "gvm_validate.h"
#include "gvm_asmutils.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <dlfcn.h>
#include <unistd.h>

#if GVM_TRACE_LOG_LEVEL > 0

# define TRACE_LOG(...) printf(__VA_ARGS__)
# define TRACE_OP(C) TRACE_LOG("> %s ", au_get_op_name(C))
# define TRACE_INT_ARG(A) TRACE_LOG("%i ", (A))
# define TRACE_NL() printf("\n");

# if GVM_TRACE_LOG_LEVEL > 1
void print_stack(val_t* stack, int stack_size) {
    printf(" stack (s:%i) | ", stack_size);
    for(int i = 0; i < stack_size; i++) {
        val_print(stack[i]);
        printf(" ");
    }
    printf("\n");
}
#  define TRACE_PRINT_STACK(S, TOP) print_stack((S), (TOP) + 1)
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
    gvm_t* VM = (gvm_t*) user;
    int offset = MEM_ADDR_TO_INDEX(addr);
    if( ADDR_IS_CONST(addr) ) {
        return VM->run.constants + offset;
    } else {
        return VM->mem.membase + offset;
    }
}

void gvm_print_val(gvm_t* vm, val_t val) {
    val_print_lookup(val, &gvm_addr_lookup, vm);
}

int gvm_get_string(gvm_t* vm, val_t val, char* dest, int dest_len) {
    return val_get_string(val, &gvm_addr_lookup, vm, dest, dest_len);
}

bool gvm_create(gvm_t* vm, int stack_size, int dyn_size) {

    int total_addressable = ( stack_size + dyn_size );
    if( total_addressable > MEM_MAX_ADDRESSABLE ) {
        printf(  "warning: the requested VM memory size %i is too large.\n"
                "\tmaximum addressable memory is %i.\n",
                total_addressable,
                MEM_MAX_ADDRESSABLE);
        return false;
    }

    int memsize = total_addressable * sizeof(val_t);
    val_t* mem = (val_t*) malloc( memsize );
    if( mem == NULL ) {
        printf("error: could'nt allocate VM memory.\n");
        return false;
    }

    // size: one bit per val_t
    uint64_t* gc_marks = (uint64_t*) malloc(CALC_GC_MARK_U64_COUNT(dyn_size) * sizeof(uint64_t));
    if( gc_marks == NULL ) {
        printf("error: could'nt allocate GC mark region.\n");
        free(mem);
        return false;
    }
    memset(gc_marks, 0, CALC_GC_MARK_U64_COUNT(dyn_size) * sizeof(uint64_t));

    vm->mem.membase = mem;
    vm->mem.memsize = total_addressable;
    
    vm->mem.stack.values = vm->mem.membase;
    vm->mem.stack.size = (int) stack_size;
    vm->mem.stack.top = -1;

    // heap & GC
    vm->mem.heap.values = vm->mem.membase + (int) stack_size;
    vm->mem.heap.size = (int) dyn_size;
    vm->mem.heap.gc_marks = gc_marks;

    env_init(&vm->env, vm);

    // assigend on execution
    vm->run = (gvm_runtime_t) { 0 };

    VALIDATION_INIT(vm);

    return true;
}

bool gvm_native_func(gvm_t* vm, char* name, int num_args, func_t func) {
    return env_add_native_func(&vm->env, name, num_args, func);
}

void gvm_destroy(gvm_t* vm) {
    if( vm == NULL || vm->mem.membase == 0 ) {
        return;
    }
    VALIDATION_DESTROY(vm);
    free(vm->mem.membase);
    free(vm->mem.heap.gc_marks);
    memset(vm, 0, sizeof(gvm_t));
}

val_t gvm_execute(gvm_t* vm, gvm_program_t* program, gvm_exec_args_t* exec_args) {

    assert(sizeof(float) == 4);
    
    val_t* stack = vm->mem.stack.values;
    val_t* consts = program->cons.buffer;
    uint8_t* instructions = program->inst.buffer;

    gvm_runtime_t* vm_run = &vm->run;
    vm_run->constants = consts;
    vm_run->instructions = instructions;
    vm_run->pc = 0;
    
    gvm_mem_t* vm_mem = &vm->mem;
    memset(vm_mem->stack.values, 0, sizeof(val_t) * vm_mem->stack.size);

    // push initial args (if any)
    vm_mem->stack.frame = -1;
    vm_mem->stack.top = -1;
    for(int i = 0; i < exec_args->args.count; i++) {
        stack[++vm_mem->stack.top] = exec_args->args.buffer[i];
    }
    
    int cycles_remaining = exec_args->cycle_limit;
    if (program->inst.size == 0) {
        cycles_remaining = 0;
    }

    while ( (cycles_remaining--) != 0 ) {

        gvm_op_t opcode = instructions[vm_run->pc++];

        TRACE_OP(opcode);

        VALIDATE_PRE(vm, opcode);

        assert(OP_OPCODE_COUNT == 32 && "Opcode count changed.");

        switch (opcode) {
            case OP_PUSH_VALUE: {
                int const_index = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(const_index);
                stack[++vm_mem->stack.top] = consts[const_index];
                vm_run->pc += 2;
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
            case OP_NEG: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_number(-a);
            } break;
            case OP_CMP_LESS_THAN: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a < b );
            } break;
            case OP_CMP_MORE_THAN: {
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a > b );
            } break;
            case OP_CMP_EQUAL: {
                const float epsilon = 0.0001f;
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( fabs(a - b) < epsilon );
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
                vm_run->pc = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(vm_run->pc);
            } break;
            case OP_JUMP_IF_FALSE: {
                TRACE_INT_ARG(READ_I16(instructions, vm_run->pc));
                if( val_into_bool(stack[vm_mem->stack.top--]) == false ) {
                    vm_run->pc = READ_I16(instructions, vm_run->pc);
                } else {
                    vm_run->pc += 2;
                }
            } break;
            case OP_HALT:{
                TRACE_NL();
                return val_number(-1002);
            } break;
            case OP_EXIT: {
                TRACE_NL();
                int return_value = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(return_value);
                return val_number(return_value);
            } break;
            case OP_CALL: {
                // push the return address
                stack[++vm_mem->stack.top] = val_number(vm_run->pc + 2);
                // jump to label / function
                vm_run->pc = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(vm_run->pc);
            } break;
            case OP_INIT: {
                // push the return address
                stack[++vm_mem->stack.top] = val_number(-1.0f);
                // jump to label / function
                vm_run->pc = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(vm_run->pc);
            } break;
            case OP_MAKE_FRAME: {

                int nargs = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(nargs);
                vm_run->pc += 2;

                int nlocals = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(nlocals);
                vm_run->pc += 2;

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
                int locals_idx = frame_start + 1 + nargs;
                for(int i = 0; i < nlocals; i++) {
                    stack[locals_idx + i] = 0;
                }

                // make room for the locals on the stack
                vm_mem->stack.top += nlocals + 1;

            } break;
            case OP_RETURN: {
                assert(vm_mem->stack.frame >= 0);

                frame_t frame = val_into_frame(stack[vm_mem->stack.frame]);
                if( frame.return_pc < 0 ) {
                    // Note: if the return address is negative we exit the vm
                    //       returning the top of stack element.
                    if( vm_mem->stack.top >= 0 ) {
                        return stack[vm_mem->stack.top];
                    } else {
                        return val_none();
                    }
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
                    if( VAL_GET_TYPE(stack[i]) == VAL_FRAME ) {
                        vm_mem->stack.frame = i;
                        break;
                    }
                }
            } break;
            case OP_PRINT: {
                gvm_print_val(vm, stack[vm_mem->stack.top--]);
            } break;
            case OP_STORE_LOCAL: {
                int local_idx = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(local_idx);
                stack[vm_mem->stack.frame + 1 + local_idx] = stack[vm_mem->stack.top--];
                vm_run->pc += 2;
            } break;
            case OP_LOAD_LOCAL: {
                int local_idx = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(local_idx);
                stack[++vm_mem->stack.top] = stack[vm_mem->stack.frame + 1 + local_idx];
                vm_run->pc += 2;
            } break;
            case OP_MAKE_ARRAY: {
                // pop array size
                val_t size = stack[vm_mem->stack.top--];
                int count = (int) val_into_number(size);
                // allocate array
                array_t array = heap_array_alloc(vm, count);
                if( ADDR_IS_NULL(array.address) ) {
                    printf("\nheap alloc failed\n");
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
                int exit_pc = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(exit_pc);
                val_t iter_val = stack[vm_mem->stack.top];
                iter_t iter = val_into_iter(iter_val);
                if( iter.remaining == 0 ) {
                    vm_mem->stack.top --;
                    vm_run->pc = exit_pc;
                } else {
                    int mem_index = MEM_ADDR_TO_INDEX(iter.current);
                    val_t value = vm_mem->membase[mem_index];
                    iter.remaining -= 1;
                    iter.current = MEM_MK_PROGR_ADDR(mem_index + 1);
                    stack[vm_mem->stack.top] = val_iter(iter);
                    stack[++vm_mem->stack.top] = value;
                    vm_run->pc += 2;
                }
            } break;
            case OP_CALL_NATIVE: {
                int native_op_name = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(native_op_name);
                func_result_t res = env_native_func_call(&vm->env,
                    consts[native_op_name],
                    &stack[vm_mem->stack.top]);
                vm_mem->stack.top -= res.arg_count;
                stack[++vm_mem->stack.top] = res.value;
                vm_run->pc += 2;
            } break;
            default: {
                char* op_str = au_get_op_name(opcode);
                printf("\nunhandled operatioin %i (%s)\n", opcode, op_str);
                return val_number(-1003);
            } break;
        }

        TRACE_NL();
        VALIDATE_POST(vm, opcode);
        TRACE_PRINT_STACK(vm_mem->stack.values, vm_mem->stack.top);
    }

    return val_number(-1004);
}

gvm_program_t gvm_program_compile_source(char* program_code) {
    return asm_assemble_code_object(program_code);
}

void gvm_program_disassemble(gvm_program_t* prog) {
    asm_debug_disassemble_code_object(prog);
}

void gvm_program_destroy(gvm_program_t* prog) {
    asm_destroy_code_object(prog);
}

gvm_program_t gvm_program_read_and_compile(char* path) {

    FILE* f = fopen(path, "r");
    
    if( f == NULL ) {
        printf("error: %s not found.\n", path);
        return (gvm_program_t) { 0 };
    }

    char *asm_code = malloc(1);
    int retry_counter = 100; 
    while( retry_counter > 0 ) {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        asm_code = realloc(asm_code, fsize + 1);
        if( fread(asm_code, fsize, 1, f) > 0 ) {
            retry_counter = -10;
            asm_code[fsize] = '\0';
        } else {
            usleep(100000);
            retry_counter --;
        }
    }

    fclose(f);

    if( retry_counter == 0 ) {
        printf("error: failed to read file: %s\n", path);
    }

    gvm_program_t obj = gvm_program_compile_source(asm_code);
    free(asm_code);

    return obj;
}