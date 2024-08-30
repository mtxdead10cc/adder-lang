#ifndef GVM_VAL_BUFFER_H_
#define GVM_VAL_BUFFER_H_

#include "gvm_types.h"

bool val_buffer_create(val_buffer_t* buffer, gvm_mem_location_t location, int capacity);
bool val_buffer_add(val_buffer_t* buffer, val_t value);
void val_buffer_destroy(val_buffer_t* buffer);
void val_buffer_print(env_t* env, val_buffer_t* buffer);

int val_buffer_find_int(val_buffer_t* buffer, int value);
int val_buffer_find_bool(val_buffer_t* buffer, bool value);
int val_buffer_find_char(val_buffer_t* buffer, char value);
int val_buffer_find_internal_string(val_buffer_t* buffer, char* chars, int len);

#endif // GVM_VAL_BUFFER_H_