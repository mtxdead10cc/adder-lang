#ifndef GVM_UTILS_H_
#define GVM_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include "gvm_value.h"

bool u8buffer_create(u8buffer_t* ub, int capacity);
bool u8buffer_write(u8buffer_t* ub, uint8_t wbyte);
void u8buffer_destroy(u8buffer_t* ub);

#endif // GVM_UTILS_H_