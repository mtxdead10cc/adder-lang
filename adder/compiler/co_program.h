#ifndef GVM_PROGRAM_H_
#define GVM_PROGRAM_H_

#include "sh_types.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>

typedef struct source_code_t {
    char* file_path;
    int   source_length;
    char* source_code;
    time_t modtime;
} source_code_t;

bool program_file_exists(char* file_path);
time_t program_file_get_modtime(char* file_path);

source_code_t program_source_read_from_file(char* filepath);
source_code_t program_source_from_memory(char* source_code, int length);

bool program_source_is_valid(source_code_t* code);
void program_source_free(source_code_t* code);

program_t program_compile(source_code_t* code, bool print_ast);

#endif // GVM_PROGRAM_H_
