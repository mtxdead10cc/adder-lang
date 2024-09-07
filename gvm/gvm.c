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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <limits.h>

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
    "OP_DUP_1",
    "OP_DUP_2",
    "OP_ROT_2",
    "OP_CMP_EQUAL",
    "OP_CMP_LESS_THAN",
    "OP_CMP_MORE_THAN",
    "OP_PUSH_VALUE",
    "OP_POP_1",
    "OP_POP_2",
    "OP_JUMP",
    "OP_JUMP_IF_FALSE",
    "OP_EXIT",
    "OP_CALL_NATIVE",
    "OP_RETURN",
    "OP_STORE",
    "OP_LOAD"
};

#if GVM_TRACE_LOG_LEVEL > 0

# define TRACE_LOG(...) printf(__VA_ARGS__)
# define TRACE_OP(C) TRACE_LOG("> %s ", ((C) >= 0 && (C) < OP_OPCODE_COUNT) ? op_names[(C)] : "<unk>")
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

bool gvm_create(gvm_t* vm, uint16_t stack_size, uint16_t dyn_size) {

    int total_addressable = ( (int) stack_size + (int) dyn_size );
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



val_t gvm_execute(gvm_t* vm, byte_code_block_t* code_obj, int max_cycles) {

    assert(sizeof(float) == 4);
    
    val_t* stack = vm->mem.stack.values;
    val_t* consts = codeblk_get_constants_ptr(code_obj);
    uint8_t* instructions = codeblk_get_instructions_ptr(code_obj);

    gvm_runtime_t* vm_run = &vm->run;
    vm_run->constants = consts;
    vm_run->instructions = instructions;
    vm_run->pc = 0;
    
    gvm_proc_mem_t* vm_mem = &vm->mem;
    vm_mem->stack.top = -1;

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
            case OP_CALL_NATIVE: {
                int const_index = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(const_index);
                func_t func = find_func(vm, consts[const_index]);
                if( func == NULL ) {
                    printf("error: failed to find native function \"");
                    gvm_print_val(vm, consts[const_index]);
                    printf("\".\n");
                } else {
                    func(vm); // function call
                }
                vm_run->pc += 2;
            } break;
            case OP_RETURN: {
                TRACE_NL();
                return stack[vm_mem->stack.top];
            } break;
            case OP_LOAD: {
                int reg_index = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(reg_index);
                stack[++vm_mem->stack.top] = vm_run->registers[reg_index];
                vm_run->pc += 2;
            } break;
            case OP_STORE: {
                int reg_index = READ_I16(instructions, vm_run->pc);
                TRACE_INT_ARG(reg_index);
                vm_run->registers[reg_index] = stack[vm_mem->stack.top--];
                vm_run->pc += 2;
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