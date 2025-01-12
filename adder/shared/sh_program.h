#ifndef SH_PROGRAM_H_
#define SH_PROGRAM_H_

#include "sh_types.h"
#include <stdio.h>
#include <stdbool.h>

typedef struct ffi_type_t ffi_type_t;

bool program_is_valid(program_t* prog);
void program_destroy(program_t* prog);
void program_disassemble(program_t* prog);


entry_point_t program_entry_point_find(program_t* prog, char* name, ift_t type);
entry_point_t program_entry_point_find_any(program_t* prog, char* name);
entry_point_t program_entry_point_find_default(program_t* prog);

bool program_entry_point_is_valid(entry_point_t entry_point);
bool program_entry_point_set_arg(entry_point_t* ep, int index, val_t arg);
void program_entry_point_set_arg_unsafe(entry_point_t* ep, int index, val_t arg);

#endif // SH_PROGRAM_H_
