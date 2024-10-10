#ifndef SH_PROGRAM_H_
#define SH_PROGRAM_H_

#include "sh_types.h"
#include <stdio.h>

void gvm_program_destroy(gvm_program_t* prog);
void gvm_program_disassemble(FILE* stream, gvm_program_t* prog);

#endif // SH_PROGRAM_H_