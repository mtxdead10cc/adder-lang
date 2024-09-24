#ifndef GVM_UTILS_H_
#define GVM_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include "gvm_value.h"

typedef struct srcref_t {
    char*   filepath;
    char*   source;
    size_t  idx_start;
    size_t  idx_end;
} srcref_t;

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


bool u8buffer_create(u8buffer_t* ub, uint32_t capacity);
void u8buffer_clear(u8buffer_t* ub);
bool u8buffer_write(u8buffer_t* ub, uint8_t wbyte);
bool u8buffer_write_multiple(u8buffer_t* ub, uint32_t count, ...);
void u8buffer_destroy(u8buffer_t* ub);

bool valbuffer_create(valbuffer_t* buffer, uint32_t capacity);
void valbuffer_clear(valbuffer_t* buffer);
bool valbuffer_add(valbuffer_t* buffer, val_t value);
void valbuffer_destroy(valbuffer_t* buffer);

bool valbuffer_find_float(valbuffer_t* buffer, float value, uint32_t* index);
bool valbuffer_find_ivec2(valbuffer_t* buffer, ivec2_t value, uint32_t* index);
bool valbuffer_find_bool(valbuffer_t* buffer, bool value, uint32_t* index);
bool valbuffer_find_char(valbuffer_t* buffer, char value, uint32_t* index);
bool valbuffer_find_string(valbuffer_t* buffer, char* chars, int len, uint32_t* index);

int string_count_until(char* text, char stopchar);
int string_parse_int(char* text, int string_length);

#define READ_U32(D, AT) ((uint32_t)((D)[(AT) + 1] << (8*3)) |\
                         (uint32_t)((D)[(AT) + 1] << (8*2)) |\
                         (uint32_t)((D)[(AT) + 1] << (8*1)) |\
                         (uint32_t)((D)[(AT)]))

uint32_t valbuffer_add_number(valbuffer_t* consts, float value);
uint32_t valbuffer_add_bool(valbuffer_t* consts, bool value);
uint32_t valbuffer_add_char(valbuffer_t* consts, char value, bool force_contiguous);
uint32_t valbuffer_add_string(valbuffer_t* consts, char* text);
uint32_t valbuffer_add_ivec2(valbuffer_t* consts, char* text);
uint32_t valbuffer_add_symbol_as_string(valbuffer_t* consts, char* text, int length);

srcref_t srcref(char* text, size_t start, size_t len, char* filepath);
srcref_t srcref_const(const char* text);
srcref_t srcref_combine(srcref_t a, srcref_t b);
size_t   srcref_len(srcref_t ref);
char*    srcref_ptr(srcref_t ref);
void     srcref_print(srcref_t ref);
bool     srcref_equals(srcref_t a, srcref_t b);

#endif // GVM_UTILS_H_