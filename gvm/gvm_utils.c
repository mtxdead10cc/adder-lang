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
            return false;
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