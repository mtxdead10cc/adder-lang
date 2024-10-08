#ifndef GVM_PROGRAM_H_
#define GVM_PROGRAM_H_

#include "shared/gvm_types.h"
#include <stdio.h>

gvm_program_t gvm_program_read_and_compile(char* path);
gvm_program_t gvm_program_compile_source(char* source, size_t source_len, char* filepath);
void gvm_program_disassemble(FILE* stream, gvm_program_t* prog);
void gvm_program_destroy(gvm_program_t* prog);

#endif // GVM_PROGRAM_H_