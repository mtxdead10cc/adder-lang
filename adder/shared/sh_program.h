#ifndef SH_PROGRAM_H_
#define SH_PROGRAM_H_

#include "sh_types.h"
#include <stdio.h>
#include <stdbool.h>

typedef struct ffi_type_t ffi_type_t;

void program_destroy(program_t* prog);
void program_disassemble(FILE* stream, program_t* prog);

entry_point_t program_get_entry_point(program_t* prog, char* name, ift_t* type, sh_msg_buffer_t* msgbuf);
bool entry_point_set_arg(entry_point_t* ep, int index, val_t arg);
void entry_point_set_arg_unsafe(entry_point_t* ep, int index, val_t arg);

#endif // SH_PROGRAM_H_
