#ifndef GVM_H_
#define GVM_H_

#include "gvm_types.h"

void test();
char* gvm_result_to_string(gvm_result_t res);
void gvm_print_if_error(gvm_result_t res, char* context);
val_t gvm_compile_and_run(char* program, bool print_dissasm);

#endif // GVM_H_