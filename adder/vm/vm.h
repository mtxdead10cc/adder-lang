#ifndef GVM_H_
#define GVM_H_

#include "sh_types.h"
#include "vm_types.h"

bool gvm_create(gvm_t* vm, int stack_size, int dyn_size);
val_t gvm_execute(gvm_t* vm, gvm_program_t* code_obj, gvm_exec_args_t* args);
void  gvm_destroy(gvm_t* vm);

void gvm_print_val(gvm_t* vm, val_t val);
int  gvm_get_string(gvm_t* vm, val_t val, char* dest, int dest_len);

#endif // GVM_H_