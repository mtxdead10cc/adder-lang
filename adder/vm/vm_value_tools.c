#include "sh_value.h"
#include "sh_utils.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>


void val_sprint(cstr_t str, val_t val) {
    switch (VAL_GET_TYPE(val))
    {
    case VAL_NUMBER:
        cstr_append_fmt(str, "%f", val_into_number(val));
        break;
    case VAL_CHAR:
        cstr_append_fmt(str, "%c", val_into_char(val));
        break;
    case VAL_BOOL:
        cstr_append_fmt(str, "%s", val_into_bool(val) ? "TRUE" : "FALSE");
        break;
    case VAL_IVEC2: {
        ivec2_t v = val_into_ivec2(val);
        cstr_append_fmt(str, "(%i, %i)", v.x, v.y);
    } break;
    case VAL_ITER: {
        iter_t v = val_into_iter(val);
        cstr_append_fmt(str, "{curr:0x%08X, rem:%i}", v.current, v.remaining);
    } break;
    case VAL_FRAME: {
        frame_t frame = val_into_frame(val);
        cstr_append_fmt(str, "<pc: %i, nargs: %i, nlocals: %i>",
            frame.return_pc,
            frame.num_args,
            frame.num_locals);
    } break;
    case VAL_ARRAY: {
        array_t a = val_into_array(val);
        cstr_append_fmt(str, "[addr: 0x%08X, len: %d]",
            a.address, a.length);
        break;
    } break;
    default:
        cstr_append_fmt(str, "<unk>");
        break;
    }
}

void val_sprint_string(cstr_t str, val_t* valbuf, int valbuf_length) {
    char* cbuf = malloc(valbuf_length + 1);
    for(int i = 0; i < valbuf_length; i++) {
        cbuf[i] = val_into_char(valbuf[i]);
    }
    cbuf[valbuf_length] = '\0';
    cstr_append_fmt(str, "%s", cbuf);
}

void val_sprint_lookup(cstr_t str, val_t val, addr_lookup_fn lookup, void* user) {
    if( VAL_GET_TYPE(val) == VAL_ARRAY && lookup != NULL && user != NULL ) {
        array_t array = val_into_array(val);
        val_t* buffer = lookup(user, array.address);
        if( buffer == NULL ) {
            cstr_append_fmt(str, "<null buffer>");
            return;
        }
        int length = array.length;
        bool is_list = VAL_GET_TYPE(buffer[0]) != VAL_CHAR;
        if(is_list) {
            cstr_append_fmt(str, "[ ");
            for(int i = 0; i < length; i++) {
                val_sprint_lookup(str, buffer[i], lookup, user);
                cstr_append_fmt(str, " ");
            }
            cstr_append_fmt(str, "]");
        } else { // string
            for(int i = 0; i < length; i++) {
                val_sprint_lookup(str, buffer[i], lookup, user);
            }
        }
    } else {
        val_sprint(str, val);
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

char* val_get_type_name(val_type_t type) {
    switch (type) {
        case VAL_ARRAY:  return "array";
        case VAL_BOOL:   return "bool";
        case VAL_CHAR:   return "char";
        case VAL_FRAME:  return "frame";
        case VAL_IVEC2:  return "ivec2";
        case VAL_NUMBER: return "number";
        default:         return "<unknown-type>";
    }
}
