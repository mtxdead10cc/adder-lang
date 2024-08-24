#ifndef GVM_VALUE_H_
#define GVM_VALUE_H_

#include "gvm_types.h"
#include <stdint.h>
#include <stdbool.h>

val_t val_number(int16_t value);
val_t val_bool(bool value);
val_t val_char(char value);
val_t val_list(val_t* start, uint16_t length);
void val_print(val_t* val);

#endif // GVM_VALUE_H_