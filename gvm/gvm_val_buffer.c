#include "gvm_val_buffer.h"
#include "gvm_value.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

bool val_buffer_create(val_buffer_t* buffer, gvm_mem_location_t location, int capacity) {
    val_t* values = (val_t*) malloc(capacity * sizeof(val_t));
    if( values == NULL ) {
        return false;
    }
    buffer->capacity = capacity;
    buffer->values = values;
    buffer->size = 0;
    buffer->storage = location;
    return true;
}

bool val_buffer_add(val_buffer_t* buffer, val_t value) {
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

void val_buffer_print(env_t* env, val_buffer_t* buffer) {
    printf("VAL BUFFER ---\n");
    for(int i = 0; i < buffer->size; i++) {
        printf("  %3i> ", i);
        val_print_env(env, &buffer->values[i]);
        printf("\n");
    }
}