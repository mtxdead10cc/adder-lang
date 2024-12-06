#ifndef GVM_PROGRAM_H_
#define GVM_PROGRAM_H_

#include "sh_types.h"
#include <stdio.h>

gvm_program_t gvm_program_read_and_compile(char* path, bool debug_print);
gvm_program_t gvm_program_compile_source(char* source, size_t source_len, char* filepath, bool debug_print);

#endif // GVM_PROGRAM_H_