#ifndef GVM_H_
#define GVM_H_

#include "gvm_types.h"


// TODO
// ASM Function calls
//  [ ] swap rows
//  [ ] swap columns
//  [ ] flood-select
//  [ ] select
//  [ ] set-grid

char* gvm_result_to_string(gvm_result_t res);
void gvm_print_if_error(gvm_result_t res, char* context);

code_object_t gvm_compile(char* program);
val_t gvm_execute(code_object_t* code_obj, int stack_size, int max_cycles);
void gvm_disassemble(code_object_t* code_obj);
void gvm_destroy(code_object_t* code_obj);

#endif // GVM_H_