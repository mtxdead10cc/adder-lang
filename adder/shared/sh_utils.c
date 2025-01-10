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
    assert(len <= SSTR_MAX_LEN);
    len = min(len, SSTR_MAX_LEN);
    strncpy(sstr.str, str, len);
    return sstr;
}

bool sstr_equal_str(sstr_t* sstr, char* str) {
    if( sstr->str[0] != str[0] )
        return false;
    size_t len = sstr_len(sstr);
    if( len != strnlen(str, SSTR_MAX_LEN) )
        return false;
    return strncmp(sstr->str, str, len) == 0;
}

bool sstr_equal(sstr_t* a, sstr_t* b) {
    if( a->str[0] != b->str[0] )
        return false;
    int len = sstr_len(a);
    if( len != sstr_len(b) )
        return false;
    return strncmp(a->str, b->str, len) == 0;
}

int sstr_len(sstr_t* sstr) {
    return strnlen(sstr->str, SSTR_MAX_LEN);
}

char* sstr_ptr(sstr_t* sstr) {
    return sstr->str;
}

void sstr_copy(sstr_t* dest, sstr_t* src) {
    memcpy(dest->str, src->str, SSTR_MAX_LEN);
}

void sstr_replace(sstr_t* sstr, char* text) {
    memcpy(sstr->str, text, strnlen(text, SSTR_MAX_LEN));
}

void sstr_clear(sstr_t* sstr) {
    memset(sstr->str, 0, SSTR_MAX_LEN);
}

int sstr_append(sstr_t* on, sstr_t* addition) {
    if( on == NULL )
        return -SSTR_MAX_LEN;
    int offs = sstr_len(on);
    int addlen = (int) sstr_len(addition);
    int remaining = SSTR_MAX_LEN - (offs + addlen);
    int len = min(SSTR_MAX_LEN - offs, addlen);
    if( len > 0 )
        memcpy(on->str + offs, addition->str, len);
    return remaining;
}

int sstr_append_str(sstr_t* on, char* addition) {
    if( on == NULL )
        return -SSTR_MAX_LEN;
    int offs = sstr_len(on);
    int addlen = strnlen(addition, SSTR_MAX_LEN);
    int remaining = SSTR_MAX_LEN - (offs + addlen);
    int len = min(SSTR_MAX_LEN - offs, addlen);
    if( len > 0 )
        memcpy(on->str + offs, addition, len);
    return remaining;
}

int sstr_append_nstr(sstr_t* on, char* addition, int addlen) {
    if( on == NULL )
        return -SSTR_MAX_LEN;
    int offs = sstr_len(on);
    int remaining = SSTR_MAX_LEN - (offs + addlen);
    int len = min(SSTR_MAX_LEN - offs, addlen);
    if( len > 0 )
        memcpy(on->str + offs, addition, addlen);
    return remaining;
}

int sstr_append_fmt(sstr_t* on, const char* fmt, ...) {
    if( on == NULL )
        return -SSTR_MAX_LEN;
    int len = (int) sstr_len(on);
    int remaining = SSTR_MAX_LEN - len;
    va_list args;
    va_start(args, fmt);
    if( remaining <= 0 ) {
        remaining -= vsnprintf(NULL, 0, fmt, args);
    } else {
        remaining -= vsnprintf(on->str + len, remaining+1, fmt, args);
    }
    va_end(args);
    return remaining;
}

int cstr_append_fmt(cstr_t str, const char* fmt, ...) {
    int len = strnlen(str.ptr, str.maxlen);
    va_list args;
    va_start(args, fmt);
    int w = vsnprintf(str.ptr + len, str.maxlen-len, fmt, args);
    va_end(args);
    return w;
}
