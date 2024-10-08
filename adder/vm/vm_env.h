#ifndef GVM_ENV_H_
#define GVM_ENV_H_

#include "sh_types.h"
#include "vm_types.h"

void env_init(env_t* env, gvm_t* gvm);
bool env_add_native_func(env_t* env, char* name, int num_args, func_t func);
void env_table_print(env_t* env);
func_result_t env_native_func_call(env_t* env, val_t func_name, val_t* stack_top);

#endif // GVM_ENVIRONMENT_H_