#include "gvm_env.h"
#include "gvm_val_buffer.h"
#include "gvm_asm.h"
#include "gvm_value.h"
#include <stdlib.h>

void fn_print(env_t* env) {
    int stack_top = env->stack.size - 1;
    val_print_env(env, &env->stack.values[stack_top]);
    printf("\n");
}


void env_init(env_t* env, byte_code_block_t* bc, int stack_size) {
    byte_code_header_t h = asm_read_byte_code_header(bc);
    int num_constants = h.const_bytes / sizeof(val_t);
    env->constants.capacity = num_constants;
    env->constants.size = num_constants;
    env->constants.storage = MEM_LOC_CONST;
    env->constants.values = (val_t*) (bc->data + h.header_size);
    val_buffer_create(&env->heap, MEM_LOC_HEAP, 5);
    val_buffer_create(&env->stack, MEM_LOC_STACK, stack_size);
    
    // set functions
    env->native.count = 0;

    // set functions
    env->native.names[env->native.count] = "print";
    env->native.funcs[env->native.count] = &fn_print;
    env->native.count ++;

}

void env_destroy(env_t* env) {
    env->constants = (val_buffer_t) { 0 };
    val_buffer_destroy(&env->heap);
    val_buffer_destroy(&env->stack);
    env->native.count = 0;
}

