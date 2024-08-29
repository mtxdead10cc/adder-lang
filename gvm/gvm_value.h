#ifndef GVM_VALUE_H_
#define GVM_VALUE_H_

#include "gvm_types.h"
#include <stdint.h>
#include <stdbool.h>

val_t val_number(int value);
val_t val_bool(bool value);
val_t val_char(char value);
val_t val_list(val_buffer_t* buffer, int start_offset, int length);
void  val_print_env(env_t* env, val_t* val);
void  val_print_mem(val_t* buffer, val_t* val);
int val_get_string(env_t* env, val_t* val, char* buffer, int max_len);

#endif // GVM_VALUE_H_