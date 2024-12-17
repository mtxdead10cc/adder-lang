#ifndef SH_PROGRAM_H_
#define SH_PROGRAM_H_

#include "sh_types.h"
#include <stdio.h>

void program_destroy(vm_program_t* prog);
void program_disassemble(FILE* stream, vm_program_t* prog);
int  program_find_entrypoint(vm_program_t* prog,
    sstr_t name,
    ffi_type_t* expected);

#endif // SH_PROGRAM_H_
