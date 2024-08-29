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

byte_code_block_t gvm_compile(char* program);
val_t gvm_execute(byte_code_block_t* code_obj, env_t* env, int max_cycles);
void gvm_disassemble(byte_code_block_t* code_obj);
void gvm_destroy(byte_code_block_t* code_obj);

#endif // GVM_H_