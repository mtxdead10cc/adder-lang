#ifndef GVM_H_
#define GVM_H_

#include "gvm_types.h"

char* gvm_result_to_string(gvm_result_t res);
void gvm_print_if_error(gvm_result_t res, char* context);

gvm_program_t gvm_program_read_and_compile(char* path);
gvm_program_t gvm_program_compile_source(char* program_code);
void gvm_program_destroy(gvm_program_t* prog);
void gvm_program_disassemble(gvm_program_t* prog);

bool  gvm_create(gvm_t* vm, int stack_size, int dyn_size);
bool gvm_native_func(gvm_t* vm, char* name, int num_args, func_t func);
val_t gvm_execute(gvm_t* vm, gvm_program_t* code_obj, gvm_exec_args_t* args);
void  gvm_destroy(gvm_t* vm);

void gvm_print_val(gvm_t* vm, val_t val);
int  gvm_get_string(gvm_t* vm, val_t val, char* dest, int dest_len);

/*



*/

#endif // GVM_H_