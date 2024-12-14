#ifndef GVM_PROGRAM_H_
#define GVM_PROGRAM_H_

#include "sh_types.h"
#include "sh_ffi.h"
#include <stdio.h>

vm_program_t program_read_and_compile(char* path, bool debug_print, ffi_host_t* bundle);
vm_program_t program_compile_source(char* source, size_t source_len, char* filepath, bool debug_print, ffi_host_t* bundle);

#endif // GVM_PROGRAM_H_
