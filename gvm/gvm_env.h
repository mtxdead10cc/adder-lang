#ifndef GVM_ENV_H_
#define GVM_ENV_H_

#include "gvm_types.h"

void env_init(env_t* env, byte_code_block_t* bc, int stack_size);
void env_print_val(env_t* env, val_t val);
int  env_get_string(env_t* env, val_t val, char* dest, int dest_len);
void env_destroy(env_t* env);

#endif // GVM_ENVIRONMENT_H_