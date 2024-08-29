#include "gvm_value.h"
#include <stdio.h>
#include "gvm_utils.h"
#include "gvm_val_buffer.h"

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
    uint32_t store = (buffer->storage & 0xF) << 28;
    uint32_t len = (length & 0x0FFFFFFF) << 14;
    uint32_t offs = (start_offset & 0x3FFF);
    return (val_t) {
        .type = VAL_LIST,
        .data.l = store | len | offs
    };
}

val_buffer_t* find_buffer(env_t* env, list_t list) {
    gvm_mem_location_t loc = GET_LIST_MEM_LOC(list);
    switch (loc) {
        case MEM_LOC_CONST: return &env->constants;
        case MEM_LOC_HEAP:  return &env->heap;
        case MEM_LOC_STACK: return &env->stack;
        default:            return NULL;
    }
}

void val_print_env(env_t* env, val_t* val) {
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
        val_buffer_t* buffer = find_buffer(env, val->data.l);
        if( buffer == NULL ) {
            printf("<null buffer>");
            break;
        }
        int length = GET_LIST_LENGTH(val->data.l);
        int offset = GET_LIST_OFFSET(val->data.l);
        bool is_list = buffer->values[offset].type != VAL_CHAR;
        if(is_list) {
            printf("[");
        }
        for(int i = 0; i < length; i++) {
            val_t* v_ptr = &buffer->values[i + offset];
            val_print_env(env, v_ptr);
            if(is_list) {
                printf(" ");
            }
        }
        if(is_list) {
            printf("]");
        }
    } break;
    default:
        printf("<unk>");
        break;
    }
}

void val_print_mem(val_t* buffer, val_t* val) {
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
        if( buffer == NULL ) {
            printf("<null buffer>");
            break;
        }
        int length = GET_LIST_LENGTH(val->data.l);
        int offset = GET_LIST_OFFSET(val->data.l);
        val_t* v_ptr = buffer + offset;
        bool is_list = v_ptr[0].type != VAL_CHAR;
        if(is_list) {
            printf("[");
        }
        for(int i = 0; i < length; i++) {
            val_print_mem(buffer, &v_ptr[i]);
            if(is_list) {
                printf(" ");
            }
        }
        if(is_list) {
            printf("]");
        }
    } break;
    default:
        printf("<unk>");
        break;
    }
}

int val_get_string(env_t* env, val_t* val, char* buffer, int max_len) {
    if( val->type != VAL_LIST ) {
        return 0;
    }
    int length = GET_LIST_LENGTH(val->data.l);
    if( length > (max_len - 1) ) {
        length = (max_len - 1);
    }
    val_buffer_t* vbuf = find_buffer(env, val->data.l);
    int offset = GET_LIST_OFFSET(val->data.l);
    for(int i = 0; i < length; i++) {
        buffer[i] = vbuf->values[offset + i].data.c;
    }
    buffer[length] = '\0';
    return length;
}
