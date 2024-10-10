#include "vm_env.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bool env_init(env_t* env, size_t capacity) {
    env->fundecls = (env_fundecl_t*) malloc(sizeof(env_fundecl_t) * capacity);
    env->handlers = (func_ptr_t*) malloc(sizeof(func_ptr_t) * capacity);
    env->argcounts = (size_t*) malloc(sizeof(size_t) * capacity);
    if( env->fundecls == NULL
     || env->handlers == NULL
     || env->argcounts == NULL )
    {
        env_destroy(env);
        return false;
    }
    env->count = 0;
    env->capacity = capacity;
    return true;
}

bool env_ensure_capacity(env_t* env, size_t additional) {
    if( env->count + additional > env->capacity ) {
        size_t new_capacity = (env->count + additional) * 2;
        env_fundecl_t* fundecls = (env_fundecl_t*) realloc(env->fundecls, sizeof(env_fundecl_t) * new_capacity);
        if( fundecls == NULL )
            return false;
        env->fundecls = fundecls;
        func_ptr_t* handlers = (func_ptr_t*) realloc(env->handlers, sizeof(func_ptr_t) * new_capacity);
        if( handlers == NULL )
            return false;
        env->handlers = handlers;
        size_t* argcounts = (size_t*) realloc(env->argcounts, sizeof(size_t) * new_capacity);
        if( argcounts == NULL )
            return false;
        env->argcounts = argcounts;
        env->capacity = new_capacity;
    }
    return true;
}

bool env_func_equals(env_fundecl_t* a, env_fundecl_t* b) {
    if( a->name[0] != a->name[0] )
        return false;
    if( a->rettype[0] != a->rettype[0] )
        return false;
    if( strncmp(a->name, b->name, GVM_DEFAULT_STRLEN) != 0 )
        return false;
    return strncmp(a->rettype, b->rettype, GVM_DEFAULT_STRLEN) == 0;
}

bool env_contains(env_t* env, env_fundecl_t* func, size_t arg_count) {
    for(size_t i = 0; i < env->count; i++) {
        if( env->argcounts[i] != arg_count )
            continue;
        if( env_func_equals(&env->fundecls[i], func) ) {
            return true;
        }
    }
    return false;
}

bool env_register_function(env_t* env, env_fundecl_t decl, size_t arg_count, func_ptr_t handler) {
    if( env_contains(env, &decl, arg_count) )
        return false;
    if( env_ensure_capacity(env, 1) == false )
        return false;
    env->fundecls[env->count] = decl;
    env->handlers[env->count] = handler;
    env->argcounts[env->count] = arg_count;
    env->count ++;
    return true;
}

void env_destroy(env_t* env) {
    if( env == NULL )
        return;
    if( env->fundecls != NULL )
        free(env->fundecls);
    if( env->handlers != NULL )
        free(env->handlers);
    if( env->argcounts != NULL )
        free(env->argcounts);
    env->fundecls = NULL;
    env->handlers = NULL;
    env->argcounts = NULL;
    env->capacity = 0;
    env->count = 0;
}

