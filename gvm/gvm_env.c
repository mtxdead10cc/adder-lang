#include "gvm_env.h"
#include "gvm.h"
#include "gvm_asm.h"
#include "gvm_value.h"
#include "gvm_config.h"
#include "gvm_utils.h"
#include "gvm_memory.h"
#include <stdlib.h>
#include <assert.h>

val_t fn_print(gvm_t* vm, val_t* args) {
    printf("\n");
    gvm_print_val(vm, args[0]);
    gvm_print_val(vm, args[1]);
    printf("\n");
    return val_none();
}

void env_init(env_t* env, gvm_t* vm) {
    env->vm = vm;
    // set functions
    env->native.count = 0;
    // set functions
    env->native.names[env->native.count] = "nprint";
    env->native.funcs[env->native.count] = &fn_print;
    env->native.argcounts[env->native.count] = 2;
    env->native.count ++;
}

int read_name(env_t* env, array_t name, char* buf, int max_len) {
    val_t* ptr = env->vm->mem.membase;
    if( MEM_IS_CONST_ADDR(name.address) ) {
        ptr = env->vm->run.constants;
    }
    ptr += MEM_ADDR_TO_INDEX(name.address);
    int length = name.length;
    if( length > (max_len - 1) ) {
        length = max_len - 1;
    }
    for(int i = 0; i < length; i++) {
        buf[i] = val_into_char(ptr[i]);
    }
    buf[length] = '\0';
    return length;
}

int env_nfunc_get_index(env_t* env, char* name) {
    // look for an entry with that name
    for(int i = 0; i < env->native.count; i++) {
        if( strcmp(env->native.names[i], name) == 0 ) {
            return i;
        }
    }
    return -1;
}

func_result_t env_nfunc_call(env_t* env, val_t func_name, val_t* stack_top) {
    char buf[GVM_ENV_NFUNC_NAME_MAX_LEN] = { 0 };
    // read name into buffer
    array_t array = val_into_array(func_name);
    assert( array.length < GVM_ENV_NFUNC_NAME_MAX_LEN && array.length > 0 );
    read_name(env, array, buf, GVM_ENV_NFUNC_NAME_MAX_LEN);
    int index = env_nfunc_get_index(env, buf);
    if( index < 0 ) {
        printf("error: native function '%s' not found.\n", buf);
        return (func_result_t) {
            .arg_count = 0,
            .value = val_none()
        };
    }
    int arg_count = env->native.argcounts[index];
    return (func_result_t) {
        .arg_count = arg_count,
        .value = env->native.funcs[index](env->vm, stack_top + 1 - arg_count)
    };
}
