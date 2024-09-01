#include "gvm_env.h"
#include "gvm_asm.h"
#include "gvm_value.h"
#include "gvm_utils.h"
#include "gvm_memory.h"
#include <stdlib.h>

val_t* env_addr_lookup(void* user, val_addr_t addr) {
    env_t* env = (env_t*) user;
    int offset = MEM_ADDR_TO_INDEX(addr);
    if( MEM_IS_CONST_ADDR(addr) ) {
        return env->constants.values + offset;
    } else {
        printf("addr_lookup memory: NOT IMPLEMENTED!");
        return NULL;
    }
}

void env_print_val(env_t* env, val_t val) {
    val_print_lookup(val, &env_addr_lookup, env);
}

int env_get_string(env_t* env, val_t val, char* dest, int dest_len) {
    return val_get_string(val, &env_addr_lookup, env, dest, dest_len);
}

void fn_print(env_t* env) {
    int stack_top = env->stack.size - 1;
    env_print_val(env, env->stack.values[stack_top]);
    printf("\n");
}


void env_init(env_t* env, byte_code_block_t* bc, int stack_size) {
    byte_code_header_t h = asm_read_byte_code_header(bc);
    int num_constants = h.const_bytes / sizeof(val_t);
    env->constants.capacity = num_constants;
    env->constants.size = num_constants;
    env->constants.values = (val_t*) (bc->data + h.header_size);
    valbuffer_create(&env->heap, 5);
    valbuffer_create(&env->stack, stack_size);
    
    // set functions
    env->native.count = 0;

    // set functions
    env->native.names[env->native.count] = "print";
    env->native.funcs[env->native.count] = &fn_print;
    env->native.count ++;

}

void env_destroy(env_t* env) {
    env->constants = (valbuffer_t) { 0 };
    valbuffer_destroy(&env->heap);
    valbuffer_destroy(&env->stack);
    env->native.count = 0;
}

