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
    size_t len = strnlen(str, SSTR_MAX_LEN+1);
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

int sstr_index_of(sstr_t* sstr, char c) {
    for(int i = 0; i < SSTR_MAX_LEN; i++) {
        if( sstr->str[i] == c )
            return i;
    }
    return -1;
}

sstr_t sstr_substr(sstr_t* sstr, int start, int end) {
    if( end > SSTR_MAX_LEN )
        end = SSTR_MAX_LEN;
    int count = end - start;
    if( count <= 0 )
        return (sstr_t) { 0 };
    sstr_t res = { 0 };
    for(int i = 0; i < count; i++) {
        res.str[i] = sstr->str[i + start];
    }
    return res;
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

bool str_is_whitespace_char(char c) {
    return c == ' '
        || c == '\t'
        || c == '\n';
}

char* str_lstrip_whitespace(char* str, int* len) {
    while( (*len) > 0 ) {
        if( str_is_whitespace_char(str[0]) ) {
            str++;
            (*len) --;
        } else {
            break;
        }
    }
    return str;
}

int str_rstrip_whitespace(char* str, int len) {
    while( len > 0 ) {
        if( str_is_whitespace_char(str[len-1]) ) {
            len --;
        } else {
            break;
        }
    }
    return len;
}

char* str_strip_whitespace(char* str, int* length) {
    str = str_lstrip_whitespace(str, length);
    *length = str_rstrip_whitespace(str, *length);
    return str;
}

bool str_is_bool(char* str, int len) {
    str = str_lstrip_whitespace(str, &len);
    if( len < 5 )
        return false;
    return strncmp("false", str, 5) == 0
        || strncmp("true", str, 4)  == 0;
}

bool str_is_float(char* str, int len) {
    str = str_lstrip_whitespace(str, &len);
    if( len <= 0 )
        return false;

    int dotcount = 0;
    int numcount = 0;
    for(int i = 0; i < len; i++) {
        if( str[i] >= '0' && str[i] <= '9' ) {
            numcount ++;
            continue;
        }
        if( str[i] == '.' && dotcount <= 1 ) {
            dotcount ++;
            continue;
        }
        if( str_is_whitespace_char(str[i]) )
            continue;
        return false;
    }

    return dotcount == 1 && numcount > 0;
}

bool str_is_int(char* str, int len) {
    str = str_lstrip_whitespace(str, &len);
    if( len <= 0 )
        return false;
    int numcount = 0;
    for(int i = 0; i < len; i++) {
        if( str[i] >= '0' && str[i] <= '9' ) {
            numcount ++;
            continue;
        }
        if( str_is_whitespace_char(str[i]) )
            continue;
        return false;
    }
    return numcount > 0;
}

bool str_is_string(char* str, int len) {
    str = str_lstrip_whitespace(str, &len);
    if( len <= 0 )
        return false;
    if( str[0] != '"' )
        return false;
    for(int i = 1; i < len; i++) {
        if( str[i] == '"' ) {
            return true;
        }
    }
    return false;
}

int str_index_of(char* str, int len, char c) {
    int index = 0;
    while (index < len) {
        if( str[index] == c ) {
            return index;
        }
        index ++;
    }
    return -1;
}
