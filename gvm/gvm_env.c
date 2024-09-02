#include "gvm_env.h"
#include "gvm.h"
#include "gvm_asm.h"
#include "gvm_value.h"
#include "gvm_utils.h"
#include "gvm_memory.h"
#include <stdlib.h>

void fn_print(gvm_t* vm) {
    int stack_top = vm->mem.stack.top;
    gvm_print_val(vm, vm->mem.stack.values[stack_top]);
    printf("\n");
}

void env_init(env_t* env, gvm_t* vm) {
    env->vm = vm;
    // set functions
    env->native.count = 0;
    // set functions
    env->native.names[env->native.count] = "print";
    env->native.funcs[env->native.count] = &fn_print;
    env->native.count ++;
}
