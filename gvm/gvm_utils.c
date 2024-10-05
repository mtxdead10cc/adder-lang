#include "gvm_utils.h"
#include "gvm.h"
#include "gvm_config.h"
#include "gvm_memory.h"
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

bool valbuffer_add(valbuffer_t* buffer, val_t value) {
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

bool valbuffer_find_float(valbuffer_t* buffer, float value, uint32_t* index) {
    for (uint32_t i = 0; i < buffer->size; i++) {
        if(VAL_GET_TYPE(buffer->values[i]) != VAL_NUMBER) {
            continue;
        }
        if(val_into_number(buffer->values[i]) == value) {
            *index = i;
            return true;
        }
    }
    return false;
}

bool valbuffer_find_ivec2(valbuffer_t* buffer, ivec2_t value, uint32_t* index) {
    for (uint32_t i = 0; i < buffer->size; i++) {
        if(VAL_GET_TYPE(buffer->values[i]) != VAL_IVEC2) {
            continue;
        }
        ivec2_t in_buffer = val_into_ivec2(buffer->values[i]);
        if( in_buffer.x == value.x && in_buffer.y == value.y ) {
            *index = i;
            return true;
        }
    }
    return false;
}

bool valbuffer_find_bool(valbuffer_t* buffer, bool value, uint32_t* index) {
    for (uint32_t i = 0; i < buffer->size; i++) {
        if(VAL_GET_TYPE(buffer->values[i]) != VAL_BOOL) {
            continue;
        }
        if( val_into_bool(buffer->values[i]) == value) {
            *index = i;
            return true;
        }
    }
    return false;
}

bool valbuffer_find_char(valbuffer_t* buffer, char value, uint32_t* index) {
    for (uint32_t i = 0; i < buffer->size; i++) {
        if(VAL_GET_TYPE(buffer->values[i]) != VAL_CHAR) {
            continue;
        }
        if( val_into_char( buffer->values[i] ) == value) {
            *index = i;
            return true;
        }
    }
    return false;
}

bool valbuffer_find_string(valbuffer_t* buffer, char* chars, int len, uint32_t* index) {
    for (uint32_t i = 0; i < buffer->size; i++) {
        if(VAL_GET_TYPE(buffer->values[i]) != VAL_ARRAY ) {
            continue;
        }
        array_t array = val_into_array(buffer->values[i]);
        int buffer_offset = MEM_ADDR_TO_INDEX(array.address);
        if( array.length != len ) {
            continue;
        }
        bool match = true;
        val_t* values = buffer->values + buffer_offset;
        for(int j = 0; j < array.length; j++) {
            val_t entry = values[j];
            if( chars[j] != val_into_char(entry) ||
                VAL_GET_TYPE(entry) != VAL_CHAR    ) 
            {
                match = false;
                break;
            }
        }
        if( match ) {
            *index = i;
            return true;
        }
    }
    return false;
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

int string_count_until(char* text, char stopchar) {
    int len = strlen(text);
    for(int i = 0; i < len; i++) {
        if( text[i] == stopchar ) {
            return i;
        }
    }
    return len;
}

int string_parse_int(char* text, int string_length) {
    char buf[string_length+1];
    for(int i = 0; i < string_length; i++) {
        buf[i] = text[i];
    }
    buf[string_length] = '\0';
    return atoi(buf);
}

uint32_t valbuffer_add_number(valbuffer_t* consts, float value) {
    uint32_t existing = 0;
    if( valbuffer_find_float(consts, value, &existing) ) {
        return existing;
    }

    if( valbuffer_add(consts, val_number(value)) == false ) {
        printf("error: consts_add_number\n");
        return consts->size;
    }
    return consts->size - 1;
}

uint32_t valbuffer_add_bool(valbuffer_t* consts, bool value) {
    uint32_t existing = 0;
    if( valbuffer_find_bool(consts, value, &existing) ) {
        return existing;
    }
    if(valbuffer_add(consts, val_bool(value)) == false) {
        printf("error: consts_add_bool\n");
        return consts->size;
    }
    return consts->size - 1;
}

uint32_t valbuffer_add_char(valbuffer_t* consts, char value, bool force_contiguous) {
    if( force_contiguous == false ) {
        uint32_t existing = 0;
        if( valbuffer_find_char(consts, value, &existing) ) {
            return existing;
        }
    }
    if(valbuffer_add(consts, val_char(value)) == false) {
        printf("error: consts_add_char\n");
        return consts->size;
    }
    return consts->size - 1;
}

uint32_t valbuffer_add_string(valbuffer_t* consts, char* text) {

    if( text[0] != '"' ) {
        printf("error: expected \" at start of string.\n");
    }

    text = text + 1;

    int in_len = string_count_until(text, '\"');
    
    // UN-ESCAPE input string
    // need this step (with malloc) to unescape '\n' etc.
    char* tmp_buffer = malloc((in_len + 1) * sizeof(char));
    int r_count = 0;
    int w_count = 0;
    while( r_count < in_len ) {
        if( text[r_count] == '\\' && (r_count + 1) < in_len ) {
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
    
    uint32_t existing = 0;
    if( valbuffer_find_string(consts, tmp_buffer, w_count, &existing) ) {
        return existing;
    }

    int string_start = consts->size;
    for(int i = 0; i < w_count; i++) {
        valbuffer_add_char(consts, tmp_buffer[i], true);
    }

    free(tmp_buffer); // free the UN-ESCAPE buffer

    bool ok = valbuffer_add(consts,
        val_array_from_args( MEM_MK_CONST_ADDR(string_start),
                  w_count));
    return ok
        ? (consts->size - 1)
        : (consts->size);
}

uint32_t valbuffer_add_ivec2(valbuffer_t* consts, char* text) {

    if( text[0] != '(' ) {
        printf("error: expected ( at start of ivec2.\n");
    }

    text = text + 1; // skip open paren

    ivec2_t value = { 0 };

    // read x
    int to_comma = string_count_until(text, ',');
    value.x = string_parse_int(text, to_comma);

    text += to_comma + 1;
    
    // read y
    int to_rparen = string_count_until(text, ')');
    value.y = string_parse_int(text, to_rparen);

    uint32_t existing = 0;
    if( valbuffer_find_ivec2(consts, value, &existing) ) {
        return existing;
    }

    bool ok = valbuffer_add(consts, val_ivec2(value));
    return ok
        ? (consts->size - 1)
        : (consts->size);
}

uint32_t valbuffer_add_symbol_as_string(valbuffer_t* consts, char* text, int length) {

    uint32_t existing = 0;
    if( valbuffer_find_string(consts, text, length, &existing)) {
        return existing;
    }

    int string_start = consts->size;
    for(int i = 0; i < length; i++) {
        valbuffer_add_char(consts, text[i], true);
    }

    bool ok = valbuffer_add(consts,
        val_array_from_args( MEM_MK_CONST_ADDR(string_start),
                  length));
    return ok
        ? (consts->size - 1)
        : (consts->size);
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
    assert(a.source == b.source && "can't combine srcres from different sources");
    return (srcref_t) {
        .idx_end    = (a.idx_end > b.idx_end)       ? a.idx_end     : b.idx_end,
        .idx_start  = (a.idx_start < b.idx_start)   ? a.idx_start   : b.idx_start,
        .source     = a.source
    };
}

size_t srcref_len(srcref_t ref) {
    return ref.idx_end - ref.idx_start;
}

char* srcref_ptr(srcref_t ref) {
    return ref.source + ref.idx_start;
}

void srcref_print(srcref_t ref) {
    size_t len = srcref_len(ref);
    char buf[len + 1];
    strncpy(buf, srcref_ptr(ref), len);
    buf[len] = '\0';
    printf("%s", buf);
}

bool srcref_equals(srcref_t a, srcref_t b) {
    size_t len = srcref_len(a);
    if( len != srcref_len(b) ) {
        return false;
    }
    char* a_str = srcref_ptr(a);
    char* b_str = srcref_ptr(b);
    return strncmp(a_str, b_str, len) == 0;
}

bool srcref_equals_string(srcref_t a, const char* b_str) {
    size_t len = srcref_len(a);
    if( len != strlen(b_str) ) {
        return false;
    }
    char* a_str = srcref_ptr(a);
    return strncmp(a_str, b_str, len) == 0;
}

bool srcref_as_float(srcref_t ref, float* value) {
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
    return snprintf(str, slen, "%.*s", (int) srcref_len(ref), srcref_ptr(ref));
}

int srcref_fprint(FILE* stream, srcref_t ref) {
    return fprintf(stream, "%.*s", (int) srcref_len(ref), srcref_ptr(ref));
}

char* srcref_tmpstr(srcref_t ref) {
    static char tmp_buffer[256] = { 0 };
    size_t len = srcref_len(ref);
    assert(len < 256 && "ref points to string that is lager than the tmp buffer.");
    snprintf(tmp_buffer, 255, "%.*s", (int) srcref_len(ref), srcref_ptr(ref));
    return tmp_buffer;
}