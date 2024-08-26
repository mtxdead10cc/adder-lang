#include "gvm_utils.h"
#include "gvm.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

bool u8buffer_create(u8buffer_t* ub, int capacity) {
    ub->data = (uint8_t*) malloc(capacity * sizeof(uint8_t));
    if( ub->data == NULL ) {
        return false;
    }
    ub->size = 0;
    ub->capacity = capacity;
    return true;
}

bool u8buffer_write(u8buffer_t* ub, uint8_t wbyte) {
    int size = ub->size;
    if( size >= ub->capacity ) {
        uint8_t* new_mem = (uint8_t*) realloc(ub->data, size * 2);
        if( new_mem == NULL ) {
            return RES_OUT_OF_MEMORY;
        }
        ub->data = new_mem;
        ub->capacity = size * 2;
    }
    ub->data[size] = wbyte;
    ub->size ++;
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


bool val_buffer_create(val_buffer_t* buffer, int capacity) {
    val_t* values = (val_t*) malloc(capacity * sizeof(val_t));
    if( values == NULL ) {
        return RES_OUT_OF_MEMORY;
    }
    buffer->capacity = capacity;
    buffer->values = values;
    buffer->size = 0;
    return RES_OK;
}

void val_buffer_internal_update_memory_location(val_t* old, val_t* current, int current_length) {
    for(int i = 0; i < current_length; i++) {
        if( current[i].type == VAL_LIST ) {
            val_t* ptr = current[i].data.l.ptr;
            int offset = (ptr - old) / sizeof(val_t);
            current[i].data.l.ptr = current + offset;
        }
    }
}

bool val_buffer_internal_add(val_buffer_t* buffer, val_t value, bool update_refs) {
    val_t* last_pointer = buffer->values;

    if( buffer->size >= buffer->capacity ) {
        int new_capacity = buffer->size * 2;
        val_t* new_vals = (val_t*) realloc(buffer->values, new_capacity * sizeof(val_t));
        if( new_vals == NULL ) {
            return RES_OUT_OF_MEMORY;
        }
        buffer->capacity = new_capacity;
        buffer->values = new_vals;
    }

    buffer->values[buffer->size] = value;
    buffer->size ++;

    if( update_refs && last_pointer != buffer->values ) {
        // update all pointers so they point to
        // the corresponding entry in the new
        // memory location
        val_buffer_internal_update_memory_location(
            last_pointer,
            buffer->values,
            buffer->size);
    }

    return RES_OK;
}

bool val_buffer_add_update_refs(val_buffer_t* buffer, val_t value) {
    return val_buffer_internal_add(buffer, value, true);
}

bool val_buffer_add(val_buffer_t* buffer, val_t value) {
    return val_buffer_internal_add(buffer, value, false);
}

void val_buffer_destroy(val_buffer_t* buffer) {
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

void val_buffer_print(val_buffer_t* buffer) {
    printf("VAL BUFFER ---\n");
    for(int i = 0; i < buffer->size; i++) {
        printf("  %3i> ", i);
        val_print(&buffer->values[i]);
        printf("\n");
    }
}