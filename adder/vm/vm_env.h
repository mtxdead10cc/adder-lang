#ifndef VM_RUNCFG_H_
#define VM_RUNCFG_H_

#include "vm_types.h"
#include "sh_types.h"

void vm_env_init(vm_env_t* env);
bool vm_env_setup(vm_env_t* env, vm_program_t* program, ffi_t* ffi);
void vm_env_destroy(vm_env_t* env);

#endif // VM_RUNCFG_H_ 