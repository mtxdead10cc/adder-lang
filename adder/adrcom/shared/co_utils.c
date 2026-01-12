#include "adrcom/shared/co_utils.h"

#include <shared/sh_utils.h>
#include <shared/sh_config.h>
#include <shared/sh_value.h>
#include <shared/sh_log.h>

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

bool val_compare(val_t a, val_t b) {
    if( a.type != b.type )
        return false;
    switch( a.type ) {
        case VAL_NONE:
            return true;
        case VAL_ARRAY:
            return (a.u.array.address == b.u.array.address)
                && (a.u.array.length == b.u.array.length);
        case VAL_BOOL:
            return a.u.boolean == b.u.boolean;
        case VAL_CHAR:
            return a.u.character == b.u.character;
        case VAL_FRAME:
            return (a.u.frame.num_args == b.u.frame.num_args)
                && (a.u.frame.num_locals == b.u.frame.num_locals)
                && (a.u.frame.return_pc == b.u.frame.return_pc);
        case VAL_ITER:
            return (a.u.iter.current == b.u.iter.current)
                && (a.u.iter.remaining == b.u.iter.remaining);
        case VAL_IVEC2:
            return (a.u.ivec.x == b.u.ivec.x)
                && (a.u.ivec.y == b.u.ivec.y);
        case VAL_NUMBER:
            return a.u.number == b.u.number;
        default:
            return false;
    }
}

bool valbuffer_linear_search(valbuffer_t* buffer, val_t match, uint32_t* index) {
    for(uint32_t i = 0; i < buffer->size; i++) {
        if( val_compare(match, buffer->values[i]) ) {
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
        sh_log_error("error: expected \" at start of string.\n");
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
                    sh_log_error("unhandled escaped character '\\%c'", next);
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


