#ifndef GVM_ENV_H_
#define GVM_ENV_H_

#include "gvm_types.h"

void env_init(env_t* env, gvm_t* gvm);
func_result_t env_nfunc_call(env_t* env, val_t func_name, val_t* stack_top);

#endif // GVM_ENVIRONMENT_H_