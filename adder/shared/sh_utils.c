#include "sh_utils.h"
#include "sh_config.h"
#include "sh_value.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

sstr_t sstr(char* str) {
    sstr_t sstr = { 0 };
    size_t len = strlen(str);
    assert(len < GVM_DEFAULT_STRLEN);
    len = min(len, GVM_DEFAULT_STRLEN);
    strncpy(sstr.str, str, len);
    return sstr;
}

bool sstr_equal_str(sstr_t* sstr, char* str) {
    if( sstr->str[0] != str[0] )
        return false;
    return strncmp(sstr->str, str, GVM_DEFAULT_STRLEN) == 0;
}

bool sstr_equal(sstr_t* a, sstr_t* b) {
    if( a->str[0] != b->str[0] )
        return false;
    return strncmp(a->str, b->str, GVM_DEFAULT_STRLEN) == 0;
}

size_t sstr_len(sstr_t* sstr) {
    return strnlen(sstr->str, GVM_DEFAULT_STRLEN);
}

char* sstr_ptr(sstr_t* sstr) {
    return sstr->str;
}

void sstr_printf(sstr_t* sstr) {
    printf("%.*s", (int)sstr_len(sstr), sstr->str);
}

void sstr_format(sstr_t* dest, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(dest->str, GVM_DEFAULT_STRLEN, fmt, ap);
    va_end(ap);
}

void sstr_copy(sstr_t* dest, sstr_t* src) {
    memcpy(dest->str, src->str, GVM_DEFAULT_STRLEN);
}