#ifndef VM_UTILS_H_
#define VM_UTILS_H_

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

#define SSTR_MAX_LEN ((int) (VM_DEFAULT_STRLEN) - 1)

sstr_t sstr(char* str);
bool   sstr_equal_str(sstr_t* sstr, char* str);
bool   sstr_equal(sstr_t* a, sstr_t* b);
int    sstr_len(sstr_t* sstr);
char*  sstr_ptr(sstr_t* sstr);
void   sstr_print(sstr_t sstr);
void   sstr_fprint(FILE* file, sstr_t* sstr);
void   sstr_copy(sstr_t* dest, sstr_t* src);
void   sstr_replace(sstr_t* sstr, char* text);
void   sstr_clear(sstr_t* sstr);

int sstr_append(sstr_t* on, sstr_t* addition);
int sstr_append_str(sstr_t* on, char* addition);
int sstr_append_nstr(sstr_t* on, char* addition, int len);
int sstr_append_fmt(sstr_t* on, const char* fmt, ...);

int cstr_append_fmt(char* on, int maxlen, const char* fmt, ...);

#endif // VM_UTILS_H_
