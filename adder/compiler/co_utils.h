#ifndef CO_UTILS_H_
#define CO_UTILS_H_

#include "co_types.h"
#include "sh_types.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

srcref_t srcref(char* text, size_t start, size_t len);
srcref_t srcref_const(const char* text);
srcref_t srcref_combine(srcref_t a, srcref_t b);
size_t   srcref_len(srcref_t ref);
char*    srcref_ptr(srcref_t ref);
bool     srcref_equals(srcref_t a, srcref_t b);
bool     srcref_equals_string(srcref_t a, const char* b_str);
bool     srcref_as_float(srcref_t ref, float* value);
bool     srcref_as_bool(srcref_t ref, bool* value);

void     srcref_print(srcref_t ref);
int      srcref_snprint(char* str, size_t slen, srcref_t ref);
int      srcref_fprint(FILE* stream, srcref_t ref);
char*    srcref_tmpstr(srcref_t ref);
sstr_t   srcref_as_sstr(srcref_t ref);

#endif // CO_UTILS_H_
