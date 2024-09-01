#ifndef GVM_H_
#define GVM_H_

#include "gvm_types.h"

char* gvm_result_to_string(gvm_result_t res);
void gvm_print_if_error(gvm_result_t res, char* context);

byte_code_block_t gvm_code_compile(char* program);
void gvm_code_destroy(byte_code_block_t* code_obj);
void gvm_code_disassemble(byte_code_block_t* code_obj);

val_t* gvm_get_constants_ptr(byte_code_block_t* code_obj);
int gvm_get_constants_count(byte_code_block_t* code_obj);

bool  gvm_create(gvm_t* vm, uint16_t stack_size, uint16_t dyn_size);
void  gvm_destroy(gvm_t* vm);
val_t gvm_execute(gvm_t* vm, byte_code_block_t* code_obj, int max_cycles);

void gvm_print_val(gvm_t* vm, val_t val);
int  gvm_get_string(gvm_t* vm, val_t val, char* dest, int dest_len);


void test();

#endif // GVM_H_