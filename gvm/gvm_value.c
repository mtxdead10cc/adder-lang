#include "gvm_value.h"
#include <stdio.h>
#include "gvm_utils.h"
#include "gvm_memory.h"

val_t val_number(int value) {
    return (val_t) {
        .type = VAL_NUMBER,
        .data.n = value
    };
}

val_t val_bool(bool value) {
    return (val_t) {
        .type = VAL_BOOL,
        .data.b = value
    };
}

val_t val_char(char value) {
    return (val_t) {
        .type = VAL_CHAR,
        .data.c = value
    };
}

val_t val_list(val_buffer_t* buffer, int start_offset, int length) {
    uint32_t id = (buffer->id & 0xF) << 28;
    uint32_t len = (length & 0x0FFFFFFF) << 14;
    uint32_t offs = (start_offset & 0x3FFF);
    return (val_t) {
        .type = VAL_LIST,
        .data.l = id | len | offs
    };
}

void val_print(val_t* val) {
    switch (val->type)
    {
    case VAL_NUMBER:
        printf("%i", val->data.n);
        break;
    case VAL_CHAR:
        printf("%c", val->data.c);
        break;
    case VAL_BOOL:
        printf("%s", val->data.b ? "TRUE" : "FALSE");
        break;
    case VAL_LIST: {
        val_buffer_t* buffer = val_buffer_find(GET_LIST_ID(val->data.l));
        if( buffer == NULL ) {
            printf("<null buffer>");
            break;
        }
        printf("[");
        int length = GET_LIST_LENGTH(val->data.l);
        int offset = GET_LIST_OFFSET(val->data.l);
        for(int i = 0; i < length; i++) {
            val_t* v_ptr = &buffer->values[i + offset];
            val_print(v_ptr);
            if(v_ptr->type != VAL_CHAR) {
                printf(" ");
            }
        }
        printf("]");
    } break;
    default:
        printf("<unk>");
        break;
    }
}
