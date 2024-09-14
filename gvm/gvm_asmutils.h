#ifndef GVM_ASMUTILS_H_
#define GVM_ASMUTILS_H_

#include "gvm_types.h"
#include "gvm_utils.h"
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>

#define COMPILER_ASSERT_MSG(COND, ...) do {\
    if(!(COND)) {\
        printf("GVM COMPILER ERROR\n\t"); printf(__VA_ARGS__); printf("\n");\
        exit(1);\
    }\
} while(false)

char* au_get_op_name(gvm_op_t opcode);
int au_get_op_instr_arg_count(gvm_op_t opcode);
void au_write_instruction(u8buffer_t* buf, int count, ...);

int au_consts_add_number(valbuffer_t* consts, float value);
int au_consts_add_bool(valbuffer_t* consts, bool value);
int au_consts_add_char(valbuffer_t* consts, char value, bool force_contiguous);
int au_consts_add_string(valbuffer_t* consts, char* text);
int au_consts_add_ivec2(valbuffer_t* consts, char* text);
int au_consts_add_symbol_as_string(valbuffer_t* consts, char* text, int length);

#define _GET_NTH_ARG(_1, _2, _3, _4, _5, N, ...) N
#define COUNT_VARARGS(...) _GET_NTH_ARG("ignored", ##__VA_ARGS__, 4, 3, 2, 1, 0)
#define au_write_instr(BUF, ...) au_write_instruction((BUF), COUNT_VARARGS(__VA_ARGS__), __VA_ARGS__)

#endif // GVM_ASMUTILS_H_