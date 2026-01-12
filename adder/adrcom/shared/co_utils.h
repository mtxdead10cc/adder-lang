#ifndef CO_UTILS_H_
#define CO_UTILS_H_

#include "adrcom/shared/co_types.h"

#include <shared/sh_types.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct srcref_t srcref_t;

typedef struct srcref_map_t {
    size_t          count;
    size_t          capacity;
    srcref_t*       key;
    bool*           is_in_use;
    uint32_t*       value;
} srcref_map_t;

typedef struct u8buffer_t {
    uint32_t size;
    uint32_t capacity;
    uint8_t* data;
} u8buffer_t;

typedef struct valbuffer_t {
    uint32_t size;
    uint32_t capacity;
    val_t* values;
} valbuffer_t;

typedef struct vb_result_t {
    bool out_of_memory;
    size_t index;
} vb_result_t;

bool u8buffer_create(u8buffer_t* ub, uint32_t capacity);
void u8buffer_clear(u8buffer_t* ub);
bool u8buffer_write(u8buffer_t* ub, uint8_t wbyte);
bool u8buffer_write_multiple(u8buffer_t* ub, uint32_t count, ...);
void u8buffer_destroy(u8buffer_t* ub);

bool valbuffer_create(valbuffer_t* buffer, uint32_t capacity);
void valbuffer_clear(valbuffer_t* buffer);
bool valbuffer_append(valbuffer_t* buffer, val_t value);
void valbuffer_destroy(valbuffer_t* buffer);

vb_result_t valbuffer_insert_int(valbuffer_t* buffer, int value);
vb_result_t valbuffer_insert_float(valbuffer_t* buffer, float value);
vb_result_t valbuffer_insert_char(valbuffer_t* buffer, char value);
vb_result_t valbuffer_insert_bool(valbuffer_t* buffer, bool value);
vb_result_t valbuffer_append_array(valbuffer_t* buffer, val_t* values, size_t count);

size_t string_count_until(char* text, char stopchar);
size_t valbuffer_sequence_from_qouted_string(char* text, val_t* result, size_t result_capacity);
void valbuffer_sequence_from_string(char* text, val_t* result, size_t length);

#endif // CO_UTILS_H_
