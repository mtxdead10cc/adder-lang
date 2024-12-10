#include "co_utils.h"
#include "sh_utils.h"
#include "sh_config.h"
#include "sh_value.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

bool u8buffer_create(u8buffer_t* ub, uint32_t capacity) {
    ub->data = (uint8_t*) malloc(capacity * sizeof(uint8_t));
    if( ub->data == NULL ) {
        return false;
    }
    ub->size = 0;
    ub->capacity = capacity;
    return true;
}

void u8buffer_clear(u8buffer_t* ub) {
    ub->size = 0;
}

bool u8buffer_ensure_capacity(u8buffer_t* ub, uint32_t additional) {
    uint32_t size = ub->size + additional;
    if( size >= ub->capacity ) {
        uint8_t* new_mem = (uint8_t*) realloc(ub->data, size * 2);
        if( new_mem == NULL ) {
            return false;
        }
        ub->data = new_mem;
        ub->capacity = size * 2;
    }
    return true;
}

bool u8buffer_write(u8buffer_t* ub, uint8_t wbyte) {
    if(u8buffer_ensure_capacity(ub, 1) == false) {
        return false;
    }
    ub->data[ub->size++] = wbyte;
    return true;
}

bool u8buffer_write_multiple(u8buffer_t* ub, uint32_t count, ...) {
    if(u8buffer_ensure_capacity(ub, count) == false) {
        return false;
    }
    va_list ap;
    va_start(ap, count);
    for(uint32_t i = 0; i < count; i++) {
        ub->data[ub->size++] = (uint8_t)(va_arg(ap, int) & 0xFF);
    }
    va_end(ap);
    return true;
}

void u8buffer_destroy(u8buffer_t* ub) {
    if( ub == NULL ) {
        return;
    }
    if( ub->data != NULL ) {
        free(ub->data);
        ub->data = NULL;
    }
    ub->capacity = 0;
    ub->size = 0;
}

bool valbuffer_create(valbuffer_t* buffer, uint32_t capacity) {
    val_t* values = (val_t*) malloc(capacity * sizeof(val_t));
    if( values == NULL ) {
        return false;
    }
    buffer->capacity = capacity;
    buffer->values = values;
    buffer->size = 0;
    return true;
}

void valbuffer_clear(valbuffer_t* buffer) {
    buffer->size = 0;
}

bool valbuffer_append(valbuffer_t* buffer, val_t value) {
    if( buffer->size >= buffer->capacity ) {
        int new_capacity = buffer->size * 2;
        val_t* new_vals = (val_t*) realloc(buffer->values, new_capacity * sizeof(val_t));
        if( new_vals == NULL ) {
            return false;
        }
        buffer->capacity = new_capacity;
        buffer->values = new_vals;
    }
    buffer->values[buffer->size] = value;
    buffer->size ++;
    return true;
}

void valbuffer_destroy(valbuffer_t* buffer) {
    if( buffer == NULL ) {
        return;
    }
    if( buffer->values != NULL ) {
        free( buffer->values );
        buffer->values = NULL;
    }
    buffer->capacity = 0;
    buffer->size = 0;
}

bool valbuffer_linear_search(valbuffer_t* buffer, val_t match, uint32_t* index) {
    for(uint32_t i = 0; i < buffer->size; i++) {
        if( match == buffer->values[i] ) {
            *index = i;
            return true;
        }
    }
    return false;
}

vb_result_t valbuffer_insert(valbuffer_t* buffer, val_t value) {
    uint32_t index = 0;
    if(valbuffer_linear_search(buffer, value, &index)) {
        return (vb_result_t) {
            .out_of_memory = false,
            .index = index
        };
    }
    if( valbuffer_append(buffer, value) == false ) {
        return (vb_result_t) {
            .out_of_memory = true,
            .index = 0
        };
    }
    return (vb_result_t) {
        .out_of_memory = false,
        .index = buffer->size - 1
    };
}

vb_result_t valbuffer_insert_int(valbuffer_t* buffer, int value) {
    return valbuffer_insert(buffer, val_number(value));
}

vb_result_t valbuffer_insert_float(valbuffer_t* buffer, float value) {
    return valbuffer_insert(buffer, val_number(value));
}

vb_result_t valbuffer_insert_char(valbuffer_t* buffer, char value) {
    return valbuffer_insert(buffer, val_char(value));
}

vb_result_t valbuffer_insert_bool(valbuffer_t* buffer, bool value) {
    return valbuffer_insert(buffer, val_bool(value));
}

vb_result_t valbuffer_append_array(valbuffer_t* buffer, val_t* sequence, size_t sequence_length) {

    uint32_t start_index = buffer->size;
    for(size_t i = 0; i < sequence_length; i++) {
        if(valbuffer_append(buffer, sequence[i]) == false) {
            return (vb_result_t) {
                .out_of_memory = true,
                .index = 0
            };
        }
    }

    val_t array_ref = val_array_from_args(
        MEM_MK_CONST_ADDR(start_index),
        (int) sequence_length );

    if( valbuffer_append(buffer, array_ref) == false ) {
        return (vb_result_t) {
            .out_of_memory = true,
            .index = 0
        };
    }

    return (vb_result_t) {
        .out_of_memory = false,
        .index = buffer->size - 1
    };
}

size_t string_count_until(char* text, char stopchar) {
    size_t len = strlen(text);
    for(size_t i = 0; i < len; i++) {
        if( text[i] == stopchar ) {
            return i;
        }
    }
    return len;
}

size_t valbuffer_sequence_from_qouted_string(char* text, val_t* result, size_t result_capacity) {

    if( text[0] != '"' ) {
        printf("error: expected \" at start of string.\n");
    }

    text = text + 1;

    size_t in_len = string_count_until(text, '\"');
    size_t str_len = min(in_len, result_capacity);
    
    // UN-ESCAPE input string
    // need this step (with malloc) to unescape '\n' etc.
    char* tmp_buffer = malloc((str_len + 1) * sizeof(char));
    size_t r_count = 0;
    size_t w_count = 0;
    while( r_count < str_len ) {
        if( text[r_count] == '\\' && (r_count + 1) < str_len ) {
            char next = text[r_count + 1];
            switch (next) {
                case 'n':
                    r_count += 2;
                    tmp_buffer[w_count++] = '\n';
                    continue; // continue next while-iteration
                case 't':
                    r_count += 2;
                    tmp_buffer[w_count++] = '\t';
                    continue; // continue next while-iteration
                case '\\':
                    r_count += 2;
                    tmp_buffer[w_count++] = '\\';
                    continue; // continue next while-iteration
                default:
                    printf("unhandled escaped character '\\%c'", next);
                    break;
            }
        }
        tmp_buffer[w_count++] = text[r_count++];
    }
    tmp_buffer[w_count] = '\0';

    for(size_t i = 0; i < w_count; i++) {
        result[i] = val_char(tmp_buffer[i]);
    }

    free(tmp_buffer); // free the UN-ESCAPE buffer
    return str_len;
}

void valbuffer_sequence_from_string(char* text, val_t* result, size_t length) {
    for(size_t i = 0; i < length; i++) {
        result[i] = val_char(text[i]);
    }
}

srcref_t srcref(char* text, size_t start, size_t len) {
    return (srcref_t) {
        .idx_start = start,
        .idx_end = start + len,
        .source = text
    };
}

srcref_t srcref_const(const char* text) {
    return (srcref_t) {
        .idx_start = 0,
        .idx_end = strlen(text),
        .source = (char*) text
    };
}

srcref_t srcref_combine(srcref_t a, srcref_t b) {
    if( a.source != b.source )
        return (srcref_t) { 0 };
    if( srcref_is_valid(a) == false )
        return b;
    if( srcref_is_valid(b) == false )
        return a;
    //assert(a.source == b.source && "can't combine srcres from different sources");
    return (srcref_t) {
        .idx_end    = (a.idx_end > b.idx_end)       ? a.idx_end     : b.idx_end,
        .idx_start  = (a.idx_start < b.idx_start)   ? a.idx_start   : b.idx_start,
        .source     = a.source
    };
}

size_t srcref_len(srcref_t ref) {
    if( ref.idx_end >= ref.idx_start )
        return ref.idx_end - ref.idx_start;
    else
        return 0;
}

char* srcref_ptr(srcref_t ref) {
    if( srcref_is_valid(ref) == false )
        return NULL;
    return ref.source + ref.idx_start;
}

void srcref_print(srcref_t ref) {
    if( srcref_is_valid(ref) ) { 
        size_t len = srcref_len(ref);
        char buf[len + 1];
        strncpy(buf, srcref_ptr(ref), len);
        buf[len] = '\0';
        printf("%s", buf);
    } else {
        printf("<invalid-srcref>");
    }
}

bool srcref_equals(srcref_t a, srcref_t b) {
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

bool srcref_equals_string(srcref_t a, const char* b_str) {
    if( srcref_is_valid(a) == false )
        return false;
    size_t len = srcref_len(a);
    if( len != strlen(b_str) )
        return false;
    char* a_str = srcref_ptr(a);
    return strncmp(a_str, b_str, len) == 0;
}

bool srcref_contains_char(srcref_t ref, char c) {
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

bool srcref_as_float(srcref_t ref, float* value) {
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

bool srcref_as_bool(srcref_t ref, bool* value) {
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

int srcref_snprint(char* str, size_t slen, srcref_t ref) {
    if( srcref_is_valid(ref) )
        return snprintf(str, slen, "%.*s", (int) srcref_len(ref), srcref_ptr(ref));
    else
        return snprintf(str, slen, "<invalid-srcref>");
}

int srcref_fprint(FILE* stream, srcref_t ref) {
    if( srcref_is_valid(ref) )
        return fprintf(stream, "%.*s", (int) srcref_len(ref), srcref_ptr(ref));
    else
        return fprintf(stream, "<invalid-srcref>");
}

sstr_t srcref_as_sstr(srcref_t ref) {
    sstr_t sstr = {0};
    if( srcref_is_valid(ref) == false )
        return sstr;
    size_t len = min(srcref_len(ref), SSTR_MAX_LEN);
    strncpy(sstr.str, srcref_ptr(ref), len);
    return sstr;
}

bool srcref_starts_with_string(srcref_t a, const char* prefix) {
    if( srcref_is_valid(a) == false )
        return false;
    return strncmp(srcref_ptr(a), prefix, strnlen(prefix, srcref_len(a))) == 0;
}

bool srcref_ends_with_string(srcref_t a, const char* suffix) {
    if( srcref_is_valid(a) == false )
        return false;
    size_t rlen = srcref_len(a);
    size_t slen = strnlen(suffix, rlen);
    size_t offs = rlen - slen;
    return strncmp(srcref_ptr(a) + offs, suffix, rlen) == 0;
}

srcref_t srcref_trim_left(srcref_t a, size_t len) {
    if(srcref_len(a) >= len)
        a.idx_start += len;
    else
        a.idx_start = a.idx_end;
    return a;
}

srcref_t srcref_trim_right(srcref_t a, size_t len) {
    if(srcref_len(a) >= len)
        a.idx_end -= len;
    else
        a.idx_end = a.idx_start;
    return a;
}