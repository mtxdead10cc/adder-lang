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

bool u8buffer_create(u8buffer_t* ub, int capacity) {
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

bool u8buffer_ensure_capacity(u8buffer_t* ub, int additional) {
    int size = ub->size + additional;
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

bool u8buffer_write_multiple(u8buffer_t* ub, int count, ...) {
    if(u8buffer_ensure_capacity(ub, count) == false) {
        return false;
    }
    va_list ap;
    va_start(ap, count);
    for(int i = 0; i < count; i++) {
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

bool valbuffer_create(valbuffer_t* buffer, int capacity) {
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

int valbuffer_find_float(valbuffer_t* buffer, float value) {
    for (int i = 0; i < buffer->size; i++) {
        if(VAL_GET_TYPE(buffer->values[i]) != VAL_NUMBER) {
            continue;
        }
        if((int) val_into_number(buffer->values[i]) == value) {
            return i;
        }
    }
    return -1;
}

int valbuffer_find_ivec2(valbuffer_t* buffer, ivec2_t value) {
    for (int i = 0; i < buffer->size; i++) {
        if(VAL_GET_TYPE(buffer->values[i]) != VAL_IVEC2) {
            continue;
        }
        ivec2_t in_buffer = val_into_ivec2(buffer->values[i]);
        if( in_buffer.x == value.x && in_buffer.y == value.y ) {
            return i;
        }
    }
    return -1;
}

int valbuffer_find_bool(valbuffer_t* buffer, bool value) {
    for (int i = 0; i < buffer->size; i++) {
        if(VAL_GET_TYPE(buffer->values[i]) != VAL_BOOL) {
            continue;
        }
        if( val_into_bool(buffer->values[i]) == value) {
            return i;
        }
    }
    return -1;
}

int valbuffer_find_char(valbuffer_t* buffer, char value) {
    for (int i = 0; i < buffer->size; i++) {
        if(VAL_GET_TYPE(buffer->values[i]) != VAL_CHAR) {
            continue;
        }
        if( val_into_char( buffer->values[i] ) == value) {
            return i;
        }
    }
    return -1;
}

int valbuffer_find_string(valbuffer_t* buffer, char* chars, int len) {
    for (int i = 0; i < buffer->size; i++) {
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
            return i;
        }
    }
    return -1;
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