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

#define READ_U32(D, AT) ((uint32_t)((D)[(AT) + 1] << (8*3)) |\
                         (uint32_t)((D)[(AT) + 1] << (8*2)) |\
                         (uint32_t)((D)[(AT) + 1] << (8*1)) |\
                         (uint32_t)((D)[(AT)]))

uint32_t au_consts_add_number(valbuffer_t* consts, float value);
uint32_t au_consts_add_bool(valbuffer_t* consts, bool value);
uint32_t au_consts_add_char(valbuffer_t* consts, char value, bool force_contiguous);
uint32_t au_consts_add_string(valbuffer_t* consts, char* text);
uint32_t au_consts_add_ivec2(valbuffer_t* consts, char* text);
uint32_t au_consts_add_symbol_as_string(valbuffer_t* consts, char* text, int length);

#endif // GVM_ASMUTILS_H_