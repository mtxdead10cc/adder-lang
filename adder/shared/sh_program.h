#ifndef SH_PROGRAM_H_
#define SH_PROGRAM_H_

#include "sh_types.h"
#include <stdio.h>
#include <stdbool.h>

typedef struct ffi_type_t ffi_type_t;

bool program_is_valid(program_t* prog);
void program_destroy(program_t* prog);
void program_disassemble(program_t* prog);

entry_point_t program_entry_point_invalid();

typedef enum pep_find_result_t {
    PEP_OK,
    PEP_INVALID_PROGRAM,                // program string was null, program has no entry points
    PEP_INVALID_PROGRAM_ENTRY_POINT,    // invalid entry point address, entry point address is not pointing to a frame
    PEP_INVALID_ENTRY_POINT_ARG_COUNT,
    PEP_NAME_NOT_FOUND,
    PEP_TYPE_NOT_MATCHING
} pep_find_result_t;

int program_entry_point_find(program_t* prog, char* name, ift_t type, entry_point_t* result);
int program_entry_point_find_any(program_t* prog, char* name, entry_point_t* result);
int program_entry_point_find_default(program_t* prog, entry_point_t* result);

bool program_entry_point_is_valid(entry_point_t entry_point);
bool program_entry_point_set_arg(entry_point_t* ep, int index, val_t arg);
void program_entry_point_set_arg_unsafe(entry_point_t* ep, int index, val_t arg);

#endif // SH_PROGRAM_H_
