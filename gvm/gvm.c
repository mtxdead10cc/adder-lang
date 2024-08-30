#include "gvm.h"
#include "gvm_grid.h"
#include "gvm_parser.h"
#include "gvm_asm.h"
#include "gvm_types.h"
#include "gvm_value.h"
#include "gvm_utils.h"
#include "gvm_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char* op_names[OP_OPCODE_COUNT] = {
    "OP_HALT",
    "OP_AND",
    "OP_OR",
    "OP_NOR",
    "OP_NOT",
    "OP_MUL",
    "OP_ADD",
    "OP_SUB",
    "OP_NEG",
    "OP_DUP",
    "OP_CMP_EQUAL",
    "OP_CMP_LESS_THAN",
    "OP_CMP_MORE_THAN",
    "OP_PUSH",
    "OP_POP",
    "OP_JUMP",
    "OP_JUMP_IF_FALSE",
    "OP_EXIT",
    "OP_CALL_NATIVE",
    "OP_RETURN"
};

#if GVM_VM_TRACE_LOG_LEVEL > 0

# define TRACE_LOG(...) printf(__VA_ARGS__)
# define TRACE_OP(C) TRACE_LOG("> %s ", ((C) >= 0 && (C) < OP_OPCODE_COUNT) ? op_names[(C)] : "<unk>")
# define TRACE_INT_ARG(A) TRACE_LOG("%i ", (A))
# define TRACE_NL() printf("\n");

# if GVM_VM_TRACE_LOG_LEVEL > 1
void print_stack(val_t* stack, int stack_size) {
    printf(" stack (s:%i) | ", stack_size);
    for(int i = 0; i < stack_size; i++) {
        val_print(&stack[i]);
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

bool symbol_equals(env_t* env, val_t* symbol, char* name) {
    if( symbol->type != VAL_LIST ) {
        return false;
    }
    int slen = strlen(name);
    int llen = GET_LIST_LENGTH(symbol->data.l);
    if( slen != llen ) {
        return false;
    }
    char buf[128] = {0};
    if(val_get_string(env, symbol, buf, 128) != llen) {
        return false;
    }
    for(int i = 0; i < slen; i++) {
        if( buf[i] != name[i] ) {
            return false;
        }
    }
    return true;
}

func_t find_func(env_t* env, val_t* symbol) {
    int count = env->native.count;
    for(int i = 0; i < count; i++) {
        if( symbol_equals(env, symbol, env->native.names[i]) ) {
            return env->native.funcs[i];
        }
    }
    return NULL;
}

val_t gvm_execute(byte_code_block_t* code_obj, env_t* env, int max_cycles) {
    
    val_t* stack = env->stack.values;
    int stack_top = -1;
    memset(stack, 0, sizeof(val_t) * env->stack.size);
    int pc = 0;
    int cycles_remaining = max_cycles;

    byte_code_header_t h = asm_read_byte_code_header(code_obj);

    if (code_obj->size == 0) {
        cycles_remaining = 0;
        max_cycles = 0;
    }

    val_t*   consts = (val_t*) (code_obj->data + h.header_size);
    uint8_t* instructions    =  code_obj->data + h.header_size + h.const_bytes;

    while ( (cycles_remaining--) != 0 ) {

        gvm_op_t opcode = instructions[pc++];

        TRACE_OP(opcode);

        switch (opcode) {
            case OP_PUSH_VALUE: {
                int const_index = READ_I16(instructions, pc);
                TRACE_INT_ARG(const_index);
                stack[++stack_top] = consts[const_index];
                pc += 2;
            } break;
            case OP_POP: {
                int num_pops = READ_I16(instructions, pc);
                TRACE_INT_ARG(num_pops);
                stack_top -= num_pops;
                pc += 2;
            } break;
            case OP_ADD: {
                val_t a = stack[stack_top--];
                val_t b = stack[stack_top--];
                stack[++stack_top] = (val_t) {
                    .type = VAL_NUMBER,
                    .data.n = a.data.n + b.data.n
                };
            } break;
            case OP_SUB: {
                val_t a = stack[stack_top--];
                val_t b = stack[stack_top--];
                stack[++stack_top] = (val_t) {
                    .type = VAL_NUMBER,
                    .data.n = a.data.n - b.data.n
                };
            } break;
            case OP_MUL: {
                val_t a = stack[stack_top--];
                val_t b = stack[stack_top--];
                stack[++stack_top] = (val_t) {
                    .type = VAL_NUMBER,
                    .data.n = a.data.n * b.data.n
                };
            } break;
            case OP_NEG: {
                val_t a = stack[stack_top--];
                stack[++stack_top] = (val_t) {
                    .type = VAL_NUMBER,
                    .data.n = -a.data.n
                };
            } break;
            case OP_CMP_LESS_THAN: {
                val_t a = stack[stack_top--];
                val_t b = stack[stack_top--];
                stack[++stack_top] = (val_t) {
                    .type = VAL_BOOL,
                    .data.b = a.data.n < b.data.n
                };
            } break;
            case OP_CMP_MORE_THAN: {
                val_t a = stack[stack_top--];
                val_t b = stack[stack_top--];
                stack[++stack_top] = (val_t) {
                    .type = VAL_BOOL,
                    .data.b = a.data.n > b.data.n
                };
            } break;
            case OP_CMP_EQUAL: {
                val_t a = stack[stack_top--];
                val_t b = stack[stack_top--];
                stack[++stack_top] = (val_t) {
                    .type = VAL_BOOL,
                    .data.b = a.data.n == b.data.n
                };
            } break;
            case OP_AND: {
                val_t a = stack[stack_top--];
                val_t b = stack[stack_top--];
                stack[++stack_top] = (val_t) {
                    .type = VAL_BOOL,
                    .data.b = a.data.b && b.data.b
                };
            } break;
            case OP_NOT: {
                val_t a = stack[stack_top--];
                val_t b = stack[stack_top--];
                stack[++stack_top] = (val_t) {
                    .type = VAL_BOOL,
                    .data.b = a.data.b && b.data.b
                };
            } break;
            case OP_DUP: {
                int n = READ_I16(instructions, pc);
                TRACE_INT_ARG(n);
                int base_index = stack_top - n + 1;
                for(int i = 0; i < n; i++) {
                    stack[++stack_top] = stack[base_index + i];
                }
                pc += 2;
            } break;
            case OP_JUMP: {
                pc = READ_I16(instructions, pc);
                TRACE_INT_ARG(pc);
            } break;
            case OP_JUMP_IF_FALSE: {
                TRACE_INT_ARG(READ_I16(instructions, pc));
                if( stack[stack_top--].data.b == false ) {
                    pc = READ_I16(instructions, pc);
                } else {
                    pc += 2;
                }
            } break;
            case OP_HALT:{
                TRACE_NL();
                return val_number(-1002);
            } break;
            case OP_EXIT: {
                TRACE_NL();
                int return_value = READ_I16(instructions, pc);
                TRACE_INT_ARG(return_value);
                return val_number(return_value);
            } break;
            case OP_CALL_NATIVE: {
                int const_index = READ_I16(instructions, pc);
                TRACE_INT_ARG(const_index);
                val_t* sym = &consts[const_index];
                func_t func = find_func(env, sym);
                if( func == NULL ) {
                    printf("error: failed to find native function \"");
                    val_print_env(env, sym);
                    printf("\".\n");
                } else {
                    // update the size of the val_buffer
                    env->stack.size = stack_top + 1;
                    // call function
                    func(env);
                    // restore stack_top
                    stack_top = env->stack.size - 1;
                }
                pc += 2;
            } break;
            case OP_RETURN: {
                TRACE_NL();
                return stack[stack_top];
            } break;
            default: {
                char* op_str = (opcode >= 0 && opcode < OP_OPCODE_COUNT)
                    ? op_names[opcode]
                    : "<unk>";
                printf("\nunknown op %i (%s)\n", opcode, op_str);
                return val_number(-1003);
            } break;
        }

        TRACE_NL();
        TRACE_PRINT_STACK(stack, stack_top);

        // update the size of the val_buffer
        env->stack.size = stack_top + 1;
    }

    return val_number(-1004);
}

byte_code_block_t gvm_compile(char* program) {
    return asm_assemble_code_object(program);
}

void gvm_disassemble(byte_code_block_t* code_obj) {
    asm_debug_disassemble_code_object(code_obj);
}

void gvm_destroy(byte_code_block_t* code_obj) {
    asm_destroy_code_object(code_obj);
}

void test() {
    grid_t grid;
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
    grid_destroy(&grid);

}