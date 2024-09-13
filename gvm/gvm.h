#ifndef GVM_H_
#define GVM_H_

#include "gvm_types.h"

char* gvm_result_to_string(gvm_result_t res);
void gvm_print_if_error(gvm_result_t res, char* context);

gvm_byte_code_t gvm_code_compile(char* program);
void gvm_code_destroy(gvm_byte_code_t* code_obj);
void gvm_code_disassemble(gvm_byte_code_t* code_obj);

bool  gvm_create(gvm_t* vm, int stack_size, int dyn_size);
bool gvm_native_func(gvm_t* vm, char* name, int num_args, func_t func);
val_t gvm_execute(gvm_t* vm, gvm_byte_code_t* code_obj, int max_cycles);
void  gvm_destroy(gvm_t* vm);

void gvm_print_val(gvm_t* vm, val_t val);
int  gvm_get_string(gvm_t* vm, val_t val, char* dest, int dest_len);

#endif // GVM_H_