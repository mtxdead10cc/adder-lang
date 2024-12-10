#ifndef SH_PROGRAM_H_
#define SH_PROGRAM_H_

#include "sh_types.h"
#include <stdio.h>

void program_destroy(vm_program_t* prog);
void program_disassemble(FILE* stream, vm_program_t* prog);

#endif // SH_PROGRAM_H_
