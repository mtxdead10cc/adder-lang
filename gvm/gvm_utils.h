#ifndef GVM_UTILS_H_
#define GVM_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include "gvm_value.h"

bool u8buffer_create(u8buffer_t* ub, uint32_t capacity);
void u8buffer_clear(u8buffer_t* ub);
bool u8buffer_write(u8buffer_t* ub, uint8_t wbyte);
bool u8buffer_write_multiple(u8buffer_t* ub, uint32_t count, ...);
void u8buffer_destroy(u8buffer_t* ub);

bool valbuffer_create(valbuffer_t* buffer, uint32_t capacity);
void valbuffer_clear(valbuffer_t* buffer);
bool valbuffer_add(valbuffer_t* buffer, val_t value);
void valbuffer_destroy(valbuffer_t* buffer);

uint32_t valbuffer_find_float(valbuffer_t* buffer, float value);
uint32_t valbuffer_find_ivec2(valbuffer_t* buffer, ivec2_t value);
uint32_t valbuffer_find_bool(valbuffer_t* buffer, bool value);
uint32_t valbuffer_find_char(valbuffer_t* buffer, char value);
uint32_t valbuffer_find_string(valbuffer_t* buffer, char* chars, int len);

int string_count_until(char* text, char stopchar);
int string_parse_int(char* text, int string_length);


#endif // GVM_UTILS_H_