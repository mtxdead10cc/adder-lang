#include "gvm.h"
#include "gvm_grid.h"
#include "gvm_parser.h"
#include "gvm_asm.h"
#include "gvm_env.h"
#include "gvm_types.h"
#include "gvm_value.h"
#include "gvm_utils.h"
#include "gvm_config.h"
#include "gvm_memory.h"
#include "gvm_heap.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <limits.h>

char* gvm_get_op_name(gvm_op_t opcode) {
    switch(opcode) {
        case OP_HALT:               return "OP_HALT";
        case OP_AND:                return "OP_AND";
        case OP_OR:                 return "OP_OR";
        case OP_NOR:                return "OP_NOR";
        case OP_NOT:                return "OP_NOT";
        case OP_MUL:                return "OP_MUL";
        case OP_ADD:                return "OP_ADD";
        case OP_SUB:                return "OP_SUB";
        case OP_NEG:                return "OP_NEG";
        case OP_DUP_1:              return "OP_DUP_1";
        case OP_DUP_2:              return "OP_DUP_2";
        case OP_ROT_2:              return "OP_ROT_2";
        case OP_CMP_EQUAL:          return "OP_CMP_EQUAL";
        case OP_CMP_LESS_THAN:      return "OP_CMP_LESS_THAN";
        case OP_CMP_MORE_THAN:      return "OP_CMP_MORE_THAN";
        case OP_PUSH_VALUE:         return "OP_PUSH_VALUE";
        case OP_POP_1:              return "OP_POP_1";
        case OP_POP_2:              return "OP_POP_2";
        case OP_JUMP:               return "OP_JUMP";
        case OP_JUMP_IF_FALSE:      return "OP_JUMP_IF_FALSE";
        case OP_EXIT_IMMEDIATE:     return "OP_EXIT_IMMEDIATE";
        case OP_EXIT_WITH_VALUE:    return "OP_EXIT_WITH_VALUE";
        case OP_CALL:               return "OP_CALL";
        case OP_MAKE_FRAME:         return "OP_MAKE_FRAME";
        case OP_RETURN:             return "OP_RETURN";
        case OP_STORE:              return "OP_STORE";
        case OP_LOAD:               return "OP_LOAD";
        case OP_PRINT:              return "OP_PRINT";
        default:                    return "<OP-UNKNOWN>";
    }
}


#if GVM_TRACE_LOG_LEVEL > 0

# define TRACE_LOG(...) printf(__VA_ARGS__)
# define TRACE_OP(C) TRACE_LOG("> %s ", gvm_get_op_name(C))
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
    if( MEM_IS_CONST_ADDR(addr) ) {
        return VM->run.constants + offset;
    } else {
        printf("addr_lookup memory: NOT IMPLEMENTED!");
        return NULL;
    }
}

void gvm_print_val(gvm_t* vm, val_t val) {
    val_print_lookup(val, &gvm_addr_lookup, vm);
}

int gvm_get_string(gvm_t* vm, val_t val, char* dest, int dest_len) {
    return val_get_string(val, &gvm_addr_lookup, vm, dest, dest_len);
}

char* gvm_result_to_string(gvm_result_t res) {
    switch (res) {
        case RES_OK:            return "OK";
        case RES_OUT_OF_MEMORY: return "OUT OF MEMORY";
        case RES_ERROR:         return "PROCEDURE ERROR";
        case RES_NOT_SUPPORTED: return "NOT SUPPORTED";
        case RES_INVALID_INPUT: return "INVALID INPUT";
        default:                return "UNKNOWN";
    }
}

void gvm_print_if_error(gvm_result_t res, char* context) {
    if( res == RES_OK ) {
        return;
    }
    if( context == (char*) 0 ) {
        printf("error: %s\n", gvm_result_to_string(res));
    } else {
        printf("error: %s -- %s\n", context, gvm_result_to_string(res));
    }
}

bool pred(type_id_t init, type_id_t curr) {
    return init == curr;
}

bool symbol_equals(gvm_t* vm, val_t symbol, char* name) {
    if( VAL_GET_TYPE(symbol) != VAL_ARRAY ) {
        return false;
    }
    int slen = strlen(name);
    int llen = val_into_array(symbol).length;
    if( slen != llen ) {
        return false;
    }
    char buf[128] = {0};
    if(gvm_get_string(vm, symbol, buf, 127) != llen) {
        return false;
    }
    for(int i = 0; i < slen; i++) {
        if( buf[i] != name[i] ) {
            return false;
        }
    }
    return true;
}

func_t find_func(gvm_t* vm, val_t symbol) {
    env_t* env = &vm->env;
    int count = env->native.count;
    for(int i = 0; i < count; i++) {
        if( symbol_equals(vm, symbol, env->native.names[i]) ) {
            return env->native.funcs[i];
        }
    }
    return NULL;
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

    return true;
}

void gvm_destroy(gvm_t* vm) {
    if( vm == NULL || vm->mem.membase == 0 ) {
        return;
    }
    free(vm->mem.membase);
    free(vm->mem.heap.gc_marks);
    memset(vm, 0, sizeof(gvm_t));
}

val_t* codeblk_get_constants_ptr(byte_code_block_t* code_obj) {
    byte_code_header_t h = asm_read_byte_code_header(code_obj);
    return (val_t*) (code_obj->data + h.header_size);
}

int codeblk_get_constants_count(byte_code_block_t* code_obj) {
    byte_code_header_t h = asm_read_byte_code_header(code_obj);
    return h.const_bytes / sizeof(val_t);
}

uint8_t* codeblk_get_instructions_ptr(byte_code_block_t* code_obj) {
    byte_code_header_t h = asm_read_byte_code_header(code_obj);
    return (code_obj->data + h.header_size + h.const_bytes);
}

int codeblk_get_instructions_count(byte_code_block_t* code_obj) {
    byte_code_header_t h = asm_read_byte_code_header(code_obj);
    return h.code_bytes / sizeof(val_t);
}


#if GVM_RUNTIME_VALIDATION > 0

char* check_stack_arg_count(gvm_t* vm, char* context, int nargs) {
    int stack_top = vm->mem.stack.top;
    if( (stack_top + 1) < nargs ) {
        static char message[257];
        snprintf(message, 256,
            "error: '%s' requres %i args, but the stack is empty.\n",
            context, nargs);
        message[256] = '\0';
        return message;
    }
    return NULL;
}

char* check_stack_args(gvm_t* vm, char* context, int arg_count, ...) {
    char* msg = check_stack_arg_count(vm, context, arg_count);
    if( msg != NULL ) {
        return msg;
    }
    va_list	arg_ptr;
    va_start(arg_ptr, arg_count);
    int stack_top = vm->mem.stack.top;
    for(int i = 0; i < arg_count; i++) {
        val_t arg = vm->mem.stack.values[stack_top - i];
        val_type_t arg_type = VAL_GET_TYPE(arg);
        val_type_t expected = va_arg(arg_ptr, val_type_t);
        if( expected != arg_type ) {
            static char message[257];
            snprintf(message, 256,
                "error: '%s' arg #%i should have been %s but was %s.\n",
                context,
                i + 1,
                val_get_type_name(expected),
                val_get_type_name(arg_type));
            message[256] = '\0';
            return message;
        }
    }
    return NULL;
}

char* check_stack(gvm_t* vm, char* context) {
    static char message[257];
    if( vm->mem.stack.top >= vm->mem.stack.size ) {
        snprintf(message, 256,
                "error: '%s' stack overflow.\n",
                context);
        message[256] = '\0';
        return message;
    }
    if( vm->mem.stack.top < 0 ) {
        snprintf(message, 256,
                "error: '%s' stack underflow.\n",
                context);
        message[256] = '\0';
        return message;
    }
    return NULL;
}

# define CHECK(C) do { if( C != NULL ) { printf("%s\n", C); assert(false); } } while(false)
# define CHECK_ARGS(VM, CTX, N, ...) CHECK(check_stack_args(VM, CTX, N, __VA_ARGS__))
# define CHECK_ARG_COUNT(VM, CTX, N) CHECK(check_stack_arg_count(VM, CTX, N))
# define CHECK_STACK(VM, CTX) CHECK(check_stack(VM, CTX))

#else

# define CHECK(C)
# define CHECK_ARGS(VM, CTX, N, ...)
# define CHECK_ARG_COUNT(VM, CTX, N)
# define CHECK_STACK(VM, CTX)

#endif

val_t gvm_execute(gvm_t* vm, byte_code_block_t* code_obj, int max_cycles) {

    assert(sizeof(float) == 4);
    
    val_t* stack = vm->mem.stack.values;
    val_t* consts = codeblk_get_constants_ptr(code_obj);
    uint8_t* instructions = codeblk_get_instructions_ptr(code_obj);

    gvm_runtime_t* vm_run = &vm->run;
    vm_run->constants = consts;
    vm_run->instructions = instructions;
    vm_run->pc = 0;
    
    gvm_mem_t* vm_mem = &vm->mem;
    vm_mem->stack.top = -1;
    vm_mem->stack.frame = 0;

    memset(vm_mem->stack.values, 0, sizeof(val_t) * vm_mem->stack.size);

    int cycles_remaining = max_cycles;

    if (code_obj->size == 0) {
        cycles_remaining = 0;
        max_cycles = 0;
    }

    while ( (cycles_remaining--) != 0 ) {

        gvm_op_t opcode = instructions[vm_run->pc++];

        TRACE_OP(opcode);

        switch (opcode) {
            case OP_PUSH_VALUE: {
                int const_index = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(const_index);
                stack[++vm_mem->stack.top] = consts[const_index];
                vm_run->pc += 2;
                CHECK_STACK(vm, "OP_PUSH_VALUE");
            } break;
            case OP_POP_1: {
                CHECK_ARG_COUNT(vm, "OP_POP_1", 1);
                vm_mem->stack.top -= 1;
                CHECK_STACK(vm, "OP_POP_1");
            } break;
            case OP_POP_2: {
                CHECK_ARG_COUNT(vm, "OP_POP_2", 2);
                vm_mem->stack.top -= 2;
                CHECK_STACK(vm, "OP_POP_2");
            } break;
            case OP_ADD: {
                CHECK_ARGS(vm, "OP_ADD", 2, VAL_NUMBER, VAL_NUMBER);
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_number(a + b);
                CHECK_STACK(vm, "OP_ADD");
            } break;
            case OP_SUB: {
                CHECK_ARGS(vm, "OP_SUB", 2, VAL_NUMBER, VAL_NUMBER);
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_number(a - b);
                CHECK_STACK(vm, "OP_SUB");
            } break;
            case OP_MUL: {
                CHECK_ARGS(vm, "OP_MUL", 2, VAL_NUMBER, VAL_NUMBER);
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_number(a * b);
                CHECK_STACK(vm, "OP_MUL");
            } break;
            case OP_NEG: {
                CHECK_ARGS(vm, "OP_NEG", 1, VAL_NUMBER);
                float a = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_number(-a);
                CHECK_STACK(vm, "OP_NEG");
            } break;
            case OP_CMP_LESS_THAN: {
                CHECK_ARGS(vm, "OP_CMP_LESS_THAN", 2, VAL_NUMBER, VAL_NUMBER);
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a < b );
                CHECK_STACK(vm, "OP_CMP_LESS_THAN");
            } break;
            case OP_CMP_MORE_THAN: {
                CHECK_ARGS(vm, "OP_CMP_MORE_THAN", 2, VAL_NUMBER, VAL_NUMBER);
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a > b );
                CHECK_STACK(vm, "OP_CMP_MORE_THAN");
            } break;
            case OP_CMP_EQUAL: {
                CHECK_ARGS(vm, "OP_CMP_EQUAL", 2, VAL_NUMBER, VAL_NUMBER);
                const float epsilon = 0.0001f;
                float a = val_into_number(stack[vm_mem->stack.top--]);
                float b = val_into_number(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( fabs(a - b) < epsilon );
                CHECK_STACK(vm, "OP_CMP_EQUAL");
            } break;
            case OP_AND: {
                CHECK_ARGS(vm, "OP_AND", 2, VAL_BOOL, VAL_BOOL);
                bool a = val_into_bool(stack[vm_mem->stack.top--]);
                bool b = val_into_bool(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a && b );
                CHECK_STACK(vm, "OP_AND");
            } break;
            case OP_OR: {
                CHECK_ARGS(vm, "OP_OR", 2, VAL_BOOL, VAL_BOOL);
                bool a = val_into_bool(stack[vm_mem->stack.top--]);
                bool b = val_into_bool(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( a || b );
                CHECK_STACK(vm, "OP_OR");
            } break;
            case OP_NOT: {
                CHECK_ARGS(vm, "OP_NOT", 1, VAL_BOOL);
                bool a = val_into_bool(stack[vm_mem->stack.top--]);
                stack[++vm_mem->stack.top] = val_bool( !a );
                CHECK_STACK(vm, "OP_NOT");
            } break;
            case OP_DUP_1: {
                CHECK_ARG_COUNT(vm, "OP_DUP_1", 1);
                val_t a = stack[vm_mem->stack.top];
                stack[++vm_mem->stack.top] = a;
            } break;
            case OP_DUP_2: {
                CHECK_ARG_COUNT(vm, "OP_DUP_2", 2);
                val_t a = stack[vm_mem->stack.top - 1];
                val_t b = stack[vm_mem->stack.top];
                stack[++vm_mem->stack.top] = a;
                stack[++vm_mem->stack.top] = b;
                CHECK_STACK(vm, "OP_DUP_2");
            } break;
            case OP_ROT_2: {
                CHECK_ARG_COUNT(vm, "OP_ROT_2", 2);
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
                CHECK_ARGS(vm, "OP_JUMP_IF_FALSE", 1, VAL_BOOL);
                TRACE_INT_ARG(READ_I16(instructions, vm_run->pc));
                if( val_into_bool(stack[vm_mem->stack.top--]) == false ) {
                    vm_run->pc = READ_I16(instructions, vm_run->pc);
                } else {
                    vm_run->pc += 2;
                }
                CHECK_STACK(vm, "OP_JUMP_IF_FALSE");
            } break;
            case OP_HALT:{
                TRACE_NL();
                return val_number(-1002);
            } break;
            case OP_EXIT_IMMEDIATE: {
                TRACE_NL();
                int return_value = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(return_value);
                return val_number(return_value);
            } break;
            case OP_EXIT_WITH_VALUE: {
                TRACE_NL();
                CHECK_ARG_COUNT(vm, "OP_EXIT_WITH_VALUE", 1);
                return stack[vm_mem->stack.top];
            } break;
            case OP_CALL: {
                // push the return address
                stack[++vm_mem->stack.top] = val_number(vm_run->pc + 2);
                CHECK_STACK(vm, "OP_CALL");
                // jump to label / function
                vm_run->pc = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(vm_run->pc);
            } break;
            case OP_MAKE_FRAME: {

                CHECK_ARGS(vm, "OP_MAKE_FRAME", 1, VAL_NUMBER);

                int nargs = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(nargs);
                vm_run->pc += 2;

                int nlocals = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(nlocals);
                vm_run->pc += 2;

                // stack top should now be the return address
                val_t call_site = stack[vm_mem->stack.top--];
                frame_t frame = (frame_t) {
                    .return_pc = (int) val_into_number(call_site),
                    .num_args = nargs
                };

                // move args to after frame and locals
                int frame_start = vm_mem->stack.top - nargs + 1;
                for(int i = nargs; i > 0; i--) {
                    stack[frame_start + nlocals + i] = stack[frame_start + i - 1];
                }

                // insert frame before locals
                stack[frame_start] = val_frame(frame);

                // init locals (not needed)
                int locals_idx = frame_start + 1;
                for(int i = 0; i < nlocals; i++) {
                    stack[locals_idx + i] = 0;
                }

                vm_mem->stack.top += nlocals + 1;
                CHECK_STACK(vm, "OP_MAKE_FRAME");

            } break;
            case OP_RETURN: {
                val_t ret_val = stack[vm_mem->stack.top];
                // skip down to call frame
                while ( VAL_GET_TYPE(stack[vm_mem->stack.top]) != VAL_FRAME ) {
                    vm_mem->stack.top --;
                    CHECK_STACK(vm, "OP_RETURN");
                }
                frame_t frame = val_into_frame(stack[vm_mem->stack.top--]);
                vm->run.pc = frame.return_pc; // resume at call site
                if( VAL_GET_TYPE(ret_val) != VAL_FRAME ) {
                    // push the return value (if any)
                    stack[++vm_mem->stack.top] = ret_val;
                }
            } break;
            case OP_PRINT: {
                CHECK_ARG_COUNT(vm, "OP_PRINT", 1);
                CHECK_STACK(vm, "OP_PRINT");
                gvm_print_val(vm, stack[vm_mem->stack.top--]);
                printf("\n");
            } break;
            case OP_LOAD: {
                int reg_index = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(reg_index);
                stack[++vm_mem->stack.top] = vm_run->registers[reg_index];
                CHECK_STACK(vm, "OP_LOAD");
                vm_run->pc += 2;
            } break;
            case OP_STORE: {
                CHECK_STACK(vm, "OP_STORE");
                int reg_index = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(reg_index);
                vm_run->registers[reg_index] = stack[vm_mem->stack.top--];
                vm_run->pc += 2;
            } break;
            default: {
                char* op_str = gvm_get_op_name(opcode);
                printf("\nunknown op %i (%s)\n", opcode, op_str);
                return val_number(-1003);
            } break;
        }

        TRACE_NL();
        TRACE_PRINT_STACK(vm_mem->stack.values, vm_mem->stack.top);
    }

    return val_number(-1004);
}

byte_code_block_t gvm_code_compile(char* program) {
    return asm_assemble_code_object(program);
}

void gvm_code_disassemble(byte_code_block_t* code_obj) {
    asm_debug_disassemble_code_object(code_obj);
}

void gvm_code_destroy(byte_code_block_t* code_obj) {
    asm_destroy_code_object(code_obj);
}

void test() {



    /*grid_t grid;
    grid_init(&grid);
    grid_fill(&grid, 1);

    grid_print(&grid);
    grid_set_size(&grid, 5, 5);
    grid_print(&grid);

    grid_set_size(&grid, 5, 12);
    grid_print(&grid);

    int buf[ 5 * 12 ] = { 0 };
    int n = grid_select(&grid, 4, 4, pred, buf);
    printf("n selected: %i\n", n);
    for(int i = 0; i < n; i++) {
        grid.data[buf[i]] = 2;
    }
    grid_print(&grid);
    grid_destroy(&grid);*/

}