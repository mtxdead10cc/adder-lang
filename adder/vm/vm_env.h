#ifndef VM_RUNCFG_H_
#define VM_RUNCFG_H_

#include "vm_types.h"
#include "sh_types.h"

bool vm_env_setup(vm_env_t* env, program_t* program, ffi_t* ffi);
bool vm_env_is_ready(vm_env_t* env);
void vm_env_destroy(vm_env_t* env);

#endif // VM_RUNCFG_H_ 