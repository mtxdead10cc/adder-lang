#ifndef GVM_UTILS_H_
#define GVM_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include "gvm_value.h"

bool u8buffer_create(u8buffer_t* ub, int capacity);
bool u8buffer_write(u8buffer_t* ub, uint8_t wbyte);
void u8buffer_destroy(u8buffer_t* ub);

bool valbuffer_create(valbuffer_t* buffer, int capacity);
bool valbuffer_add(valbuffer_t* buffer, val_t value);
void valbuffer_destroy(valbuffer_t* buffer);

int valbuffer_find_int(valbuffer_t* buffer, int value);
int valbuffer_find_bool(valbuffer_t* buffer, bool value);
int valbuffer_find_char(valbuffer_t* buffer, char value);
int valbuffer_find_string(valbuffer_t* buffer, char* chars, int len);


#endif // GVM_UTILS_H_