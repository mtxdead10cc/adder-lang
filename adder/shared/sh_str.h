#ifndef STR_H_
#define STR_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>


inline static bool str_is_whitespace_char(char c) {
    return c == ' '
        || c == '\t'
        || c == '\n';
}

inline static char* str_lstrip_whitespace(char* str, int* len) {
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

inline static int str_rstrip_whitespace(char* str, int len) {
    while( len > 0 ) {
        if( str_is_whitespace_char(str[len-1]) ) {
            len --;
        } else {
            break;
        }
    }
    return len;
}

inline static char* str_strip_whitespace(char* str, int* length) {
    str = str_lstrip_whitespace(str, length);
    *length = str_rstrip_whitespace(str, *length);
    return str;
}

inline static bool str_is_bool(char* str, int len) {
    str = str_lstrip_whitespace(str, &len);
    if( len < 5 )
        return false;
    return strncmp("false", str, 5) == 0
        || strncmp("true", str, 4)  == 0;
}

inline static bool str_is_float(char* str, int len) {
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

inline static bool str_is_int(char* str, int len) {
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

inline static bool str_is_string(char* str, int len) {
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

inline static int str_index_of(char* str, int len, char c) {
    int index = 0;
    while (index < len) {
        if( str[index] == c ) {
            return index;
        }
        index ++;
    }
    return -1;
}

#define STR_MIN(a, b) ((a) < (b) ? (a) : (b))

////////////// CSTR ////////////////

typedef struct cstr_t {
    char* ptr;
    int maxlen;
} cstr_t;

#define define_cstr(name, size)  \
    char   name##buf[size] = {0};\
    cstr_t name = (cstr_t){.ptr=name##buf, .maxlen=size};

inline static int cstr_append_fmt(cstr_t str, const char* fmt, ...) {
    int len = strnlen(str.ptr, str.maxlen);
    va_list args;
    va_start(args, fmt);
    int w = vsnprintf(str.ptr + len, str.maxlen-len, fmt, args);
    va_end(args);
    return w;
}

////////////// SSTR ////////////////

#define SSTR_MAX_STR_MEM 128

typedef struct sstr_t {
    char str[SSTR_MAX_STR_MEM];
} sstr_t;

#define SSTR_MAX_LEN ((int) (SSTR_MAX_STR_MEM) - 1)


inline static sstr_t sstr(char* str) {
    sstr_t sstr = { 0 };
    size_t len = strnlen(str, SSTR_MAX_LEN+1);
    assert(len <= SSTR_MAX_LEN);
    len = STR_MIN(len, SSTR_MAX_LEN);
    strncpy(sstr.str, str, len);
    return sstr;
}

inline static int sstr_len(sstr_t* sstr) {
    return strnlen(sstr->str, SSTR_MAX_LEN);
}

inline static bool sstr_equal_str(sstr_t* sstr, char* str) {
    if( sstr->str[0] != str[0] )
        return false;
    size_t len = sstr_len(sstr);
    if( len != strnlen(str, SSTR_MAX_LEN) )
        return false;
    return strncmp(sstr->str, str, len) == 0;
}

inline static bool sstr_equal(sstr_t* a, sstr_t* b) {
    if( a->str[0] != b->str[0] )
        return false;
    int len = sstr_len(a);
    if( len != sstr_len(b) )
        return false;
    return strncmp(a->str, b->str, len) == 0;
}

inline static char* sstr_ptr(sstr_t* sstr) {
    return sstr->str;
}

inline static void sstr_copy(sstr_t* dest, sstr_t* src) {
    memcpy(dest->str, src->str, SSTR_MAX_LEN);
}

inline static void sstr_replace(sstr_t* sstr, char* text) {
    memcpy(sstr->str, text, strnlen(text, SSTR_MAX_LEN));
}

inline static void sstr_clear(sstr_t* sstr) {
    memset(sstr->str, 0, SSTR_MAX_LEN);
}

inline static int sstr_index_of(sstr_t* sstr, char c) {
    for(int i = 0; i < SSTR_MAX_LEN; i++) {
        if( sstr->str[i] == c )
            return i;
    }
    return -1;
}

inline static sstr_t sstr_substr(sstr_t* sstr, int start, int end) {
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

inline static int sstr_append(sstr_t* on, sstr_t* addition) {
    if( on == NULL )
        return -SSTR_MAX_LEN;
    int offs = sstr_len(on);
    int addlen = (int) sstr_len(addition);
    int remaining = SSTR_MAX_LEN - (offs + addlen);
    int len = STR_MIN(SSTR_MAX_LEN - offs, addlen);
    if( len > 0 )
        memcpy(on->str + offs, addition->str, len);
    return remaining;
}

inline static int sstr_append_str(sstr_t* on, char* addition) {
    if( on == NULL )
        return -SSTR_MAX_LEN;
    int offs = sstr_len(on);
    int addlen = strnlen(addition, SSTR_MAX_LEN);
    int remaining = SSTR_MAX_LEN - (offs + addlen);
    int len = STR_MIN(SSTR_MAX_LEN - offs, addlen);
    if( len > 0 )
        memcpy(on->str + offs, addition, len);
    return remaining;
}

inline static int sstr_append_nstr(sstr_t* on, char* addition, int addlen) {
    if( on == NULL )
        return -SSTR_MAX_LEN;
    int offs = sstr_len(on);
    int remaining = SSTR_MAX_LEN - (offs + addlen);
    int len = STR_MIN(SSTR_MAX_LEN - offs, addlen);
    if( len > 0 )
        memcpy(on->str + offs, addition, addlen);
    return remaining;
}

inline static int sstr_append_fmt(sstr_t* on, const char* fmt, ...) {
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


////////////// SRCREF ////////////////

typedef struct srcref_t {
    char*   source;
    size_t  idx_start;
    size_t  idx_end;
} srcref_t;

#define srcref_is_valid(REF) ((REF).source != NULL)

inline static srcref_t srcref(char* text, size_t start, size_t len) {
    return (srcref_t) {
        .idx_start = start,
        .idx_end = start + len,
        .source = text
    };
}

inline static srcref_t srcref_const(const char* text) {
    return (srcref_t) {
        .idx_start = 0,
        .idx_end = strlen(text),
        .source = (char*) text
    };
}

inline static srcref_t srcref_combine(srcref_t a, srcref_t b) {
    if( srcref_is_valid(a) == false )
        return b;
    if( srcref_is_valid(b) == false )
        return a;
    if( a.source != b.source )
        return (srcref_t) { 0 };
    //assert(a.source == b.source && "can't combine srcres from different sources");
    return (srcref_t) {
        .idx_end    = (a.idx_end > b.idx_end)       ? a.idx_end     : b.idx_end,
        .idx_start  = (a.idx_start < b.idx_start)   ? a.idx_start   : b.idx_start,
        .source     = a.source
    };
}

inline static size_t srcref_len(srcref_t ref) {
    if( ref.idx_end >= ref.idx_start )
        return ref.idx_end - ref.idx_start;
    else
        return 0;
}

inline static char* srcref_ptr(srcref_t ref) {
    if( srcref_is_valid(ref) == false )
        return NULL;
    return ref.source + ref.idx_start;
}


inline static void srcref_sprint(cstr_t str, srcref_t ref) {
    if( srcref_is_valid(ref) ) { 
        size_t len = srcref_len(ref);
        char buf[len + 1];
        strncpy(buf, srcref_ptr(ref), len);
        buf[len] = '\0';
        cstr_append_fmt(str, "%s", buf);
    } else {
        cstr_append_fmt(str, "<invalid-srcref>");
    }
}

inline static bool srcref_equals(srcref_t a, srcref_t b) {
    if( srcref_is_valid(a) == false
     || srcref_is_valid(b) == false )
        return false;
    size_t len = srcref_len(a);
    if( len != srcref_len(b) )
        return false;
    char* a_str = srcref_ptr(a);
    char* b_str = srcref_ptr(b);
    return strncmp(a_str, b_str, len) == 0;
}

inline static bool srcref_equals_string(srcref_t a, const char* b_str) {
    if( srcref_is_valid(a) == false )
        return false;
    size_t len = srcref_len(a);
    if( len != strlen(b_str) )
        return false;
    char* a_str = srcref_ptr(a);
    return strncmp(a_str, b_str, len) == 0;
}

inline static bool srcref_contains_char(srcref_t ref, char c) {
    if( srcref_is_valid(ref) == false )
        return false;
    size_t len = srcref_len(ref);
    char* ptr = srcref_ptr(ref);
    for(size_t i = 0; i < len; i++) {
        if(ptr[i] == c)
            return true;
    }
    return false;
}

inline static bool srcref_as_float(srcref_t ref, float* value) {
    if( srcref_is_valid(ref) == false )
        return false;
    size_t len = srcref_len(ref);
    char buf[len+1];
    strncpy(buf, ref.source + ref.idx_start, len);
    buf[len] = '\0';
    if( sscanf(buf, "%f", value) > 0 ) {
        return true;
    }
    return false;
}

inline static bool srcref_as_bool(srcref_t ref, bool* value) {
    if( srcref_is_valid(ref) == false )
        return false;
    if(srcref_equals_string(ref, "true")) {
        *value = true;
        return true;
    } else if(srcref_equals_string(ref, "false")) {
        *value = false;
        return true;
    }
    return false;
}

inline static int srcref_snprint(char* str, size_t slen, srcref_t ref) {
    if( srcref_is_valid(ref) )
        return snprintf(str, slen, "%.*s", (int) srcref_len(ref), srcref_ptr(ref));
    else
        return snprintf(str, slen, "<invalid-srcref>");
}

inline static sstr_t srcref_as_sstr(srcref_t ref) {
    sstr_t sstr = {0};
    if( srcref_is_valid(ref) == false )
        return sstr;
    size_t len = STR_MIN(srcref_len(ref), SSTR_MAX_LEN);
    strncpy(sstr.str, srcref_ptr(ref), len);
    return sstr;
}

inline static bool srcref_starts_with_string(srcref_t a, const char* prefix) {
    if( srcref_is_valid(a) == false )
        return false;
    return strncmp(srcref_ptr(a), prefix, strnlen(prefix, srcref_len(a))) == 0;
}

inline static bool srcref_ends_with_string(srcref_t a, const char* suffix) {
    if( srcref_is_valid(a) == false )
        return false;
    size_t rlen = srcref_len(a);
    size_t slen = strnlen(suffix, rlen);
    size_t offs = rlen - slen;
    return strncmp(srcref_ptr(a) + offs, suffix, rlen) == 0;
}

inline static srcref_t srcref_trim_left(srcref_t a, size_t len) {
    if(srcref_len(a) >= len)
        a.idx_start += len;
    else
        a.idx_start = a.idx_end;
    return a;
}

inline static srcref_t srcref_trim_right(srcref_t a, size_t len) {
    if(srcref_len(a) >= len)
        a.idx_end -= len;
    else
        a.idx_end = a.idx_start;
    return a;
}



#endif // STR_H_