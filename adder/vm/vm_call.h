#ifndef VM_CALL_H_
#define VM_CALL_H_

#include "vm_types.h"
#include "sh_types.h"

void vm_call_init(vm_call_t* inv, vm_program_t* program);
bool vm_call_set_arg_count(vm_call_t* inv, int arg_count);
bool vm_call_set_arg(vm_call_t* inv, int arg_index, val_t val);
bool vm_call_set_entry(vm_call_t* inv, int index);
void vm_call_set_entry_unchecked(vm_call_t* inv, uint32_t address, int argcount);
bool vm_call_lookup_entry(vm_call_t* inv, char* name, ffi_type_t* type_decr);
bool vm_call_validate(vm_call_t* inv);


#endif // VM_CALL_H_ 