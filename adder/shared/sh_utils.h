#ifndef GVM_UTILS_H_
#define GVM_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "sh_types.h"
#include <stdarg.h>

#define READ_U32(D, AT) ((uint32_t)((0xFF & (D)[(AT) + 3]) << (8*3)) |\
                         (uint32_t)((0xFF & (D)[(AT) + 2]) << (8*2)) |\
                         (uint32_t)((0xFF & (D)[(AT) + 1]) << (8*1)) |\
                         (uint32_t)((0xFF & (D)[(AT) + 0]) << (8*0)))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define clamp(v, a, b) (max(min((v), b), a))

sstr_t sstr(char* str);
bool   sstr_equal_str(sstr_t* sstr, char* str);
bool   sstr_equal(sstr_t* a, sstr_t* b);
size_t sstr_len(sstr_t* sstr);
char*  sstr_ptr(sstr_t* sstr);
void   sstr_format(sstr_t* dest, const char* fmt, ...);
void   sstr_printf(sstr_t* sstr);

#endif // GVM_UTILS_H_