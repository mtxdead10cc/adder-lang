#ifndef GVM_MEMORY_H
#define GVM_MEMORY_H

#include "gvm_types.h"

val_buffer_t* val_buffer_create(int capacity);
bool val_buffer_add(val_buffer_t* buffer, val_t value);
void val_buffer_destroy(val_buffer_t* buffer);
void val_buffer_print(val_buffer_t* buffer);
val_buffer_t* val_buffer_find(int id);

#endif // GVM_MEMORY_H