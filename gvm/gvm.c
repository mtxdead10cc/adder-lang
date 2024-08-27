#include "gvm.h"
#include "gvm_grid.h"
#include "gvm_parser.h"
#include "gvm_asm.h"
#include "gvm_types.h"
#include "gvm_value.h"
#include <stdio.h>

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

#define TRACE_LOG_LEVEL 0

#if TRACE_LOG_LEVEL > 0

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
    "OP_JUMP",
    "OP_JUMP_IF_FALSE",
    "OP_EXIT",
    "OP_RETURN"
};

# define TRACE_LOG(...) printf(__VA_ARGS__)
# define TRACE_OP(C) TRACE_LOG("> %s ", ((C) >= 0 && (C) < OP_OPCODE_COUNT) ? op_names[(C)] : "<unk>")
# define TRACE_INT_ARG(A) TRACE_LOG("%i ", (A))
# define TRACE_NL() printf("\n");

# if TRACE_LOG_LEVEL > 1
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

val_t gvm_execute(code_object_t* code_obj, int stack_size, int max_cycles) {

    val_t stack[stack_size];
    int stack_top = -1;
    memset(stack, 0, sizeof(val_t) * stack_size);
    int pc = 0;
    int cycles_remaining = max_cycles;
    
    if( code_obj->code.instr == NULL ||
        code_obj->constants == NULL )
    {
        cycles_remaining = 0;
        max_cycles = 0;
    }

    while ( (cycles_remaining--) != 0 ) {

        gvm_op_t opcode = code_obj->code.instr[pc++];

        TRACE_OP(opcode);

        switch (opcode)
        {
        case OP_PUSH: {
            int const_index = READ_I16(code_obj->code.instr, pc);
            TRACE_INT_ARG(const_index);
            stack[++stack_top] = code_obj->constants->values[const_index];
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
            int n = READ_I16(code_obj->code.instr, pc);
            TRACE_INT_ARG(n);
            int base_index = stack_top - n + 1;
            for(int i = 0; i < n; i++) {
                stack[++stack_top] = stack[base_index + i];
            }
            pc += 2;
        } break;
        case OP_JUMP: {
            pc = READ_I16(code_obj->code.instr, pc);
            TRACE_INT_ARG(pc);
        } break;
        case OP_JUMP_IF_FALSE: {
            TRACE_INT_ARG(READ_I16(code_obj->code.instr, pc));
            if( stack[stack_top--].data.b == false ) {
                pc = READ_I16(code_obj->code.instr, pc);
            } else {
                pc += 2;
            }
        } break;
        case OP_HALT:{
            return val_number(-1002);
        } break;
        case OP_EXIT: {
            int return_value = READ_I16(code_obj->code.instr, pc);
            TRACE_INT_ARG(return_value);
            return val_number(return_value);
        } break;
        case OP_RETURN: {
            return stack[stack_top];
        } break;
        default:
            printf("unknown op %i\n", opcode);
            return val_number(-1003);
        }

        TRACE_NL();
        TRACE_PRINT_STACK(stack, stack_top);
    }

    return val_number(-1004);
}

code_object_t gvm_compile(char* program) {
    return asm_assemble_code_object(program);
}

void gvm_disassemble(code_object_t* code_obj) {
    asm_debug_disassemble_code_object(code_obj);
}

void gvm_destroy(code_object_t* code_obj) {
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