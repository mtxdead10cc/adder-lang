#ifndef VM_ENV_H_
#define VM_ENV_H_

#include "vm_types.h"
#include "sh_types.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static inline env_fundecl_t env_mk_func(char* name, char* return_type) {
    env_fundecl_t envfunc = (env_fundecl_t) { 0 };
    strncpy(envfunc.name, name, GVM_DEFAULT_STRLEN);
    strncpy(envfunc.rettype, return_type, GVM_DEFAULT_STRLEN);
    return envfunc;
}

bool env_init(env_t* env, size_t capacity);
bool env_register_function(env_t* env, env_fundecl_t decl, size_t arg_count, func_ptr_t handler);
void env_destroy(env_t* env);

#endif // VM_ENV_H_