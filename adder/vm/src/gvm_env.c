#include "vm/gvm_env.h"
#include "vm/gvm.h"
#include "shared/gvm_value.h"
#include "shared/gvm_config.h"
#include "shared/gvm_utils.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

void env_init(env_t* env, gvm_t* vm) {
    env->vm = vm;
    env->table = (func_table_t) { 0 };
}

typedef struct hash_key_t {
    char* key;
    int   key_len;
    uint  hash_code;
} hash_key_t;

hash_key_t str_hash(char* key) {
    hash_key_t result = { 0 };
    int len = strnlen(key, GVM_ENV_NFUNC_NAME_MAX_LEN-1);
    uint hash_code = len + 5;
    for(int i = 0; i < len; i++) {
        hash_code += (hash_code + key[i]) * 7919U;
    }
    result.key = key;
    result.key_len = len;
    result.hash_code = hash_code;
    return result;
}

hash_key_t val_hash(val_t* key, int len) {
    static char reuse_buf[GVM_ENV_NFUNC_NAME_MAX_LEN];
    hash_key_t result = { 0 };
    int max_len = GVM_ENV_NFUNC_NAME_MAX_LEN;
    len = (len < (max_len - 1))
        ? len
        : (max_len - 1);
    uint hash_code = len + 5;
    for(int i = 0; i < len; i++) {
        char c = val_into_char(key[i]);
        hash_code += (hash_code + c) * 7919U;
        reuse_buf[i] = c;
    }
    reuse_buf[len] = '\0';
    result.hash_code = hash_code;
    result.key = reuse_buf;
    result.key_len = len;
    return result;
}

bool env_table_insert(env_t* env, hash_key_t hk, tabval_t val) {
    int start_index = hk.hash_code % GVM_ENV_NFUNC_TABLE_SIZE;
    for(int i = 0; i < GVM_ENV_NFUNC_TABLE_SIZE; i++) {
        int tab_index = (i + start_index) % GVM_ENV_NFUNC_TABLE_SIZE;
        if( env->table.is_in_use[tab_index] == false ) {
            memcpy(env->table.key[tab_index], hk.key, hk.key_len * sizeof(char));
            env->table.value[tab_index] = val;
            env->table.is_in_use[tab_index] = true;
            return true;
        } else if (strncmp(env->table.key[tab_index], hk.key, hk.key_len) == 0) {
            printf("error: native function duplicate name '%s' already added ('%s').\n",
                env->table.key[tab_index], hk.key);
            return false;
        }
    }
    return false;
}

void env_table_print(env_t* env) {
    for(int i = 0; i < GVM_ENV_NFUNC_TABLE_SIZE; i++) {
        char* entry_key = env->table.key[i];
        printf("%i > ", i);
        if( entry_key == NULL ) {
            printf(" NULL\n");
        } else {
            printf(" '%s'\n", entry_key);
        }
    }
}

tabval_t* env_table_lookup(env_t* env, hash_key_t key) {
    int start_index = key.hash_code % GVM_ENV_NFUNC_TABLE_SIZE;
    for(int i = 0; i < GVM_ENV_NFUNC_TABLE_SIZE; i++) {
        int tab_index = (i + start_index) % GVM_ENV_NFUNC_TABLE_SIZE;
        char* entry_key = env->table.key[tab_index];
        if( strncmp(entry_key, key.key, key.key_len) == 0 ) {
            return &env->table.value[tab_index];
        }
    }
    return NULL;
}

bool env_add_native_func(env_t* env, char* name, int num_args, func_t func) {
    tabval_t val = (tabval_t) {
        .func = func,
        .argc = num_args
    };
    return env_table_insert(env, str_hash(name), val);
}

func_result_t env_native_func_call(env_t* env, val_t func_name, val_t* stack_top) {
    array_t key = val_into_array(func_name);
    assert( key.length < GVM_ENV_NFUNC_NAME_MAX_LEN && key.length > 0 );
    val_t* ptr = env->vm->mem.membase;
    if( ADDR_IS_CONST(key.address) ) {
        ptr = env->vm->run.constants;
    }
    ptr += MEM_ADDR_TO_INDEX(key.address);
    hash_key_t hk = val_hash(ptr, key.length);
    tabval_t* entry = env_table_lookup(env, hk);
    if( entry == NULL ) {
        printf("error: failed to find key='%s' in table.\n",
            hk.key);
        return (func_result_t) {
            .arg_count = 0,
            .value = val_none()
        };
    } else {
        return (func_result_t) {
            .arg_count = entry->argc,
            .value = entry->func(env->vm, stack_top + 1 - entry->argc)
        };
    }
}
