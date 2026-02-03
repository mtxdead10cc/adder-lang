#ifndef SH_SRC_H__
#define SH_SRC_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "shared/sh_str.h"

typedef struct src_t {
    size_t  path_length;
    size_t  buff_length;
    char*   path;       // pointer to path string (in dyndata)
    char*   buff;       // pointer to text buffer (in dyndata)
    char    dyndata[];  // trailing data section
} src_t;

typedef struct srcref_t {
    src_t*  src;
    size_t  idx_start;
    size_t  idx_end;
} srcref_t;

src_t* src_create(char* name, char* buffer, size_t buffer_length);
src_t* src_create_const(char* name, const char* buffer);
src_t* src_load(void* file_path);
void   src_destroy(src_t* src);

srcref_t src_linsearch(src_t* src, const char* pattern);

////////////// SRCREF ////////////////



#define srcref_is_valid(REF) ((REF).src != NULL)

inline static srcref_t srcref(src_t* src, size_t start, size_t len) {
    return (srcref_t) {
        .idx_start = start,
        .idx_end = start + len,
        .src = src
    };
}

inline static srcref_t srcref_full_range(src_t* src) {
    return (srcref_t) {
        .idx_start = 0,
        .idx_end = src->buff_length,
        .src = src
    };
}

inline static srcref_t srcref_combine(srcref_t a, srcref_t b) {
    if( srcref_is_valid(a) == false )
        return b;
    if( srcref_is_valid(b) == false )
        return a;
    if( a.src != b.src )
        return (srcref_t) { 0 };
    //assert(a.source == b.source && "can't combine srcres from different sources");
    return (srcref_t) {
        .idx_end    = (a.idx_end > b.idx_end)       ? a.idx_end     : b.idx_end,
        .idx_start  = (a.idx_start < b.idx_start)   ? a.idx_start   : b.idx_start,
        .src     = a.src
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
    return ref.src->buff + ref.idx_start;
}


inline static void srcref_sprint(cstr_t str, srcref_t ref) {
    if( srcref_is_valid(ref) ) { 
        size_t len = srcref_len(ref);
        char buf[len + 1];
        strncpy(buf, srcref_ptr(ref), len);
        buf[len] = '\0';
        cstr_append_fmt(&str, "%s", buf);
    } else {
        cstr_append_fmt(&str, "<invalid-srcref>");
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

inline static bool srcref_equals_string_n(srcref_t a, char* b_str, size_t b_len) {
    if( srcref_is_valid(a) == false )
        return false;
    size_t len = srcref_len(a);
    if( len != b_len )
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
    strncpy(buf, ref.src->buff + ref.idx_start, len);
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

typedef struct srcloc_t {
    size_t line;
    size_t column;
    char*  path;
    size_t path_length;
} srcloc_t;

inline static srcloc_t srcref_get_location(srcref_t a) {

    if( srcref_is_valid(a) == false )
        return (srcloc_t) { 0 };

    srcloc_t loc = (srcloc_t) {
        .column = 1,
        .line = 1,
        .path = a.src->path,
        .path_length = a.src->path_length
    };

    size_t stop = a.src->buff_length;
    if( stop > a.idx_end )
        stop = a.idx_end;

    for(size_t i = 0; i < stop; i++) {
        if(a.src->buff[i] == '\n') {
            loc.line += 1;
            loc.column = 1;
        } else {
            loc.column += 1;
        }
    }

    return loc;
}

#endif // SH_SRC_H__