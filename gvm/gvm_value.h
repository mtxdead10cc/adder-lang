#ifndef GVM_VALUE_H_
#define GVM_VALUE_H_

#include "gvm_types.h"
#include <stdint.h>
#include <stdbool.h>

#define VAL_MK_TYPE_ID(T) ( ((val_t)(T) & 0xFF) << 56 )
#define VAL_MK_NUMBER(VAL) (VAL_MK_TYPE_ID(VAL_NUMBER) | (((val_t)(VAL)) & 0xFFFFFFFF))
#define VAL_MK_BOOL(VAL) (VAL_MK_TYPE_ID(VAL_BOOL) | ((VAL) ? 0xFF : 0x00))
#define VAL_MK_CHAR(VAL) (VAL_MK_TYPE_ID(VAL_CHAR) | (((val_t)(VAL)) & 0xFFFF))
#define VAL_MK_LIST(ADDR, LEN) ( VAL_MK_TYPE_ID(VAL_LIST)\
                            | (  (val_t)(((val_t)(LEN) & 0xFFFF) << 16) )\
                            | (  (val_t)( (val_t)(ADDR) & 0xFFFF      ) ) )
#define VAL_GET_TYPE(V)     ((val_type_t)(((val_t)(V)) >> 56))
#define VAL_GET_NUMBER(V)   ((float)     (((val_t)(V))          & 0xFFFFFFFF))
#define VAL_GET_BOOL(V)     ((bool)     ((((val_t)(V))          & 0xFF) > 0x80))
#define VAL_GET_CHAR(V)     ((char)      (((val_t)(V))          & 0xFFFF))
#define VAL_GET_LIST_LENGTH(V) (uint16_t)  (((val_t)(V) >> 16)    & 0xFFFF) 
#define VAL_GET_LIST_ADDR(V)   (val_addr_t)( (val_t)(V)           & 0xFFFF )  

val_t val_number(int value);
val_t val_bool(bool value);
val_t val_char(char value);
val_t val_list(val_addr_t address, uint16_t length);

void val_print(val_t val);
void val_print_lookup(val_t val, addr_lookup_fn lookup, void* user);
int val_get_string(val_t val, addr_lookup_fn lookup, void* user, char* dest, int dest_len);
void val_print_lookup_val_array(val_t* lookup_buffer, val_t val);
int val_get_string_val_array(val_t* lookup_buffer, val_t val, char* dest, int dest_len);

#endif // GVM_VALUE_H_