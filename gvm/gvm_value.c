#include "gvm_value.h"
#include <stdio.h>

val_t val_number(int16_t value) {
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

val_t val_list(val_t* start, uint16_t length) {
    return (val_t) {
        .type = VAL_LIST,
        .data.l = (list_t) {
            .length = length,
            .ptr = start
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
        for(int i = 0; i < val->data.l.length; i++) {
            val_print(val->data.l.ptr + i);
            printf(" ");
        }
        printf("]");
        break;
    default:
        printf("<unknown-value>");
        break;
    }
}
