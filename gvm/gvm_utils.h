#ifndef GVM_UTILS_H_
#define GVM_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include "gvm_value.h"

bool u8buffer_create(u8buffer_t* ub, int capacity);
bool u8buffer_write(u8buffer_t* ub, uint8_t wbyte);
void u8buffer_destroy(u8buffer_t* ub);

bool val_buffer_create(val_buffer_t* buffer, int capacity);
bool val_buffer_add(val_buffer_t* buffer, val_t value);
void val_buffer_destroy(val_buffer_t* buffer);
void val_buffer_print(val_buffer_t* buffer);

#endif // GVM_UTILS_H_