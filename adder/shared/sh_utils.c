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
    size_t len = sstr_len(sstr);
    if( len != strnlen(str, GVM_DEFAULT_STRLEN) )
        return false;
    return strncmp(sstr->str, str, len) == 0;
}

bool sstr_equal(sstr_t* a, sstr_t* b) {
    if( a->str[0] != b->str[0] )
        return false;
    size_t len = sstr_len(a);
    if( len != sstr_len(b) )
        return false;
    return strncmp(a->str, b->str, len) == 0;
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

void sstr_copy(sstr_t* dest, sstr_t* src) {
    memcpy(dest->str, src->str, GVM_DEFAULT_STRLEN);
}

void sstr_replace(sstr_t* sstr, char* text) {
    memcpy(sstr->str, text, strnlen(text, GVM_DEFAULT_STRLEN));
}

void sstr_clear(sstr_t* sstr) {
    memset(sstr->str, 0, GVM_DEFAULT_STRLEN);
}

void sstr_append(sstr_t* on, sstr_t* addition) {
    size_t offs = sstr_len(on);
    if( offs == GVM_DEFAULT_STRLEN )
        return;
    size_t len = min(GVM_DEFAULT_STRLEN - offs, sstr_len(addition));
    memcpy(on->str + offs, addition->str, len);
}

void sstr_append_str(sstr_t* on, char* addition) {
    size_t offs = sstr_len(on);
    if( offs == GVM_DEFAULT_STRLEN )
        return;
    size_t len = min(
        GVM_DEFAULT_STRLEN - offs,
        strnlen(addition, GVM_DEFAULT_STRLEN));
    memcpy(on->str + offs, addition, len);
}

void sstr_append_nstr(sstr_t* on, char* addition, size_t len) {
    size_t offs = sstr_len(on);
    if( offs == GVM_DEFAULT_STRLEN )
        return;
    len = min(GVM_DEFAULT_STRLEN - offs, len);
    memcpy(on->str + offs, addition, len);
}
