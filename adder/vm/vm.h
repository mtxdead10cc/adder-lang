#ifndef GVM_H_
#define GVM_H_

#include "sh_types.h"
#include "vm_types.h"

bool vm_create(vm_t* vm, int stack_size, int dyn_size);
val_t vm_execute(vm_t* vm, vm_env_t* env, entry_point_t* ep, program_t* program);
void  vm_destroy(vm_t* vm);

void vm_print_val(vm_t* vm, val_t val);
int  vm_get_string(vm_t* vm, val_t val, char* dest, int dest_len);

#endif // GVM_H_
