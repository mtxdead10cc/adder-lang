#include "gvm_memory.h"
#include "gvm_value.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_MEM_BUFFFERS 16

// TODO: MEMORY CAN'T be val_buffer unless
// they are also allocated on the heap!!!!!!!!

typedef struct global_memory_t {
    val_buffer_t buffers[MAX_MEM_BUFFFERS];
    bool used[MAX_MEM_BUFFFERS];
    int buffer_count;
} global_memory_t;

static global_memory_t global_memory = { 0 };

int alloc_buffer_id() {
    for (int i = 0; i < MAX_MEM_BUFFFERS; i++) {
        if(global_memory.used[i] == false) {
            global_memory.used[i] = true;
            global_memory.buffers[i].id = i;
            return i;
        }
    }
    return -1;
}

void return_buffer_id(int id) {
    if( id < 0 || id >= MAX_MEM_BUFFFERS ) {
        return;
    }
    global_memory.used[id] = false; 
}

val_buffer_t* val_buffer_find(int id) {
    if( id < 0 || id >= MAX_MEM_BUFFFERS ) {
        return NULL;
    }
    return &global_memory.buffers[id];
}

val_buffer_t* val_buffer_create(int capacity) {
    if( global_memory.buffer_count >= MAX_MEM_BUFFFERS ) {
        printf("The maximum memory buffers threashold (%i) was reached.\n",
            global_memory.buffer_count);
        return false;
    }
    int id = alloc_buffer_id();
    val_buffer_t* buffer = val_buffer_find(id);
    val_t* values = (val_t*) malloc(capacity * sizeof(val_t));
    if( values == NULL ) {
        return_buffer_id(id);
        return NULL;
    }
    buffer->capacity = capacity;
    buffer->values = values;
    buffer->size = 0;
    return buffer;
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
    return_buffer_id(buffer->id);
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