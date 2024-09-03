#include "gvm_value.h"
#include <stdio.h>
#include "gvm_utils.h"
#include "gvm_memory.h"
 
val_t val_number(int value) {
    return VAL_MK_NUMBER(value);
}

val_t val_bool(bool value) {
    return VAL_MK_BOOL(value);
}

val_t val_char(char value) {
    return VAL_MK_CHAR(value);
}

val_t val_array(uint16_t address, uint16_t length) {
    return VAL_MK_ARRAY(address, length);
}

void val_print(val_t val) {
    switch (VAL_GET_TYPE(val))
    {
    case VAL_NUMBER:
        printf("%f", VAL_GET_NUMBER(val));
        break;
    case VAL_CHAR:
        printf("%c", VAL_GET_CHAR(val));
        break;
    case VAL_BOOL:
        printf("%s", VAL_GET_BOOL(val) ? "TRUE" : "FALSE");
        break;
    case VAL_ARRAY: {
        val_addr_t addr = VAL_GET_ARRAY_ADDR(val);
        printf("<ref: 0x%04X (%d), len: %d>",
            addr, addr,
            VAL_GET_ARRAY_LENGTH(val));
        break;
    } break;
    default:
        printf("<unk>");
        break;
    }
}

void val_print_lookup(val_t val, addr_lookup_fn lookup, void* user) {
    if( VAL_GET_TYPE(val) == VAL_ARRAY && lookup != NULL && user != NULL ) {
        val_t* buffer = lookup(user, VAL_GET_ARRAY_ADDR(val));
        if( buffer == NULL ) {
            printf("<null buffer>");
            return;
        }
        int length = VAL_GET_ARRAY_LENGTH(val);
        bool is_list = VAL_GET_TYPE(buffer[0]) != VAL_CHAR;
        if(is_list) {
            printf("[");
        }
        for(int i = 0; i < length; i++) {
            val_print_lookup(buffer[i], lookup, user);
            if(is_list) {
                printf(" ");
            }
        }
        if(is_list) {
            printf("]");
        }
    } else {
        val_print(val);
    }
}

int val_get_string(val_t val, addr_lookup_fn lookup, void* user, char* dest, int dest_len) {
    if( lookup == NULL || VAL_GET_TYPE(val) != VAL_ARRAY ) {
        return 0;
    }
    int length = VAL_GET_ARRAY_LENGTH(val);
    if( length > (dest_len - 1) ) {
        length = (dest_len - 1);
    }
    val_t* vbuf = lookup(user, VAL_GET_ARRAY_ADDR(val));
    for(int i = 0; i < length; i++) {
        dest[i] = VAL_GET_CHAR(vbuf[i]);
    }
    dest[length] = '\0';
    return length;
}

val_t* lookup_single_buffer(void* user, val_addr_t addr) {
    if( user == NULL ) {
        return NULL;
    }
    val_t* base = (val_t*) user;
    int offset = MEM_ADDR_TO_INDEX(addr);
    return base + offset;
}

void val_print_lookup_val_array(val_t* lookup_buffer, val_t val) {
    val_print_lookup(val, &lookup_single_buffer, lookup_buffer);
}

int val_get_string_val_array(val_t* lookup_buffer, val_t val, char* dest, int dest_len) {
    return val_get_string(val, &lookup_single_buffer, lookup_buffer, dest, dest_len);
}