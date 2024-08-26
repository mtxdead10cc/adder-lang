#include "gvm_value.h"
#include <stdio.h>
#include "gvm_utils.h"

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

val_t val_list(val_buffer_t* buffer, uint16_t start_index, uint16_t length) {
    return (val_t) {
        .type = VAL_LIST,
        .data.l = (list_t) {
            .length = length,
            .start_index = start_index,
            .buffer = buffer
        }
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
    case VAL_LIST:
        printf("[");
        val_buffer_t* buffer = val->data.l.buffer;
        int start = val->data.l.start_index;
        int length = val->data.l.length;
        val_t* list = &buffer->values[start];
        for(int i = 0; i < length; i++) {
            val_print(&list[i]);
            if(list[i].type != VAL_CHAR) {
                printf(" ");
            }
        }
        printf("]");
        break;
    default:
        printf("<unk>");
        break;
    }
}
