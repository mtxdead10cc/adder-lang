#include "gvm_value.h"
#include <stdio.h>
#include "gvm_utils.h"
#include "gvm_memory.h"

void val_print(val_t val) {
    switch (VAL_GET_TYPE(val))
    {
    case VAL_NUMBER:
        printf("%f", val_into_number(val));
        break;
    case VAL_CHAR:
        printf("%c", val_into_char(val));
        break;
    case VAL_BOOL:
        printf("%s", val_into_bool(val) ? "TRUE" : "FALSE");
        break;
    case VAL_IVEC2: {
        ivec2_t v = val_into_ivec2(val);
        printf("(%i, %i)", v.x, v.y);
    } break;
    case VAL_ARRAY: {
        array_t a = val_into_array(val);
        printf("<ref: 0x%04X (%d), len: %d>",
            a.address, a.address, a.length);
        break;
    } break;
    default:
        printf("<unk>");
        break;
    }
}

void val_print_lookup(val_t val, addr_lookup_fn lookup, void* user) {
    if( VAL_GET_TYPE(val) == VAL_ARRAY && lookup != NULL && user != NULL ) {
        array_t array = val_into_array(val);
        val_t* buffer = lookup(user, array.address);
        if( buffer == NULL ) {
            printf("<null buffer>");
            return;
        }
        int length = array.length;
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
    array_t array = val_into_array(val);
    int length = array.length;
    if( length > (dest_len - 1) ) {
        length = (dest_len - 1);
    }
    val_t* vbuf = lookup(user, array.address);
    for(int i = 0; i < length; i++) {
        dest[i] = val_into_char(vbuf[i]);
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