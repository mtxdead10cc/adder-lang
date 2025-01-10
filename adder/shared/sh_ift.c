#include "sh_ift.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "sh_utils.h"
#include "sh_log.h"

ift_t ift_void(void) {
    ift_t type = { 0 };
    type.count = 1;
    type.tags[0] = IFT_VOID;
    return type;
}

ift_t ift_int(void) {
    ift_t type = { 0 };
    type.count = 1;
    type.tags[0] = IFT_I32;
    return type;
}

ift_t ift_float(void) {
    ift_t type = { 0 };
    type.count = 1;
    type.tags[0] = IFT_F32;
    return type;
}

ift_t ift_char(void) {
    ift_t type = { 0 };
    type.count = 1;
    type.tags[0] = IFT_CHAR;
    return type;
}

ift_t ift_bool(void) {
    ift_t type = { 0 };
    type.count = 1;
    type.tags[0] = IFT_BOOL;
    return type;
}

ift_t ift_unknown(void) {
    ift_t type = { 0 };
    type.count = 1;
    type.tags[0] = IFT_UNK;
    return type;
}

ift_t ift_combine(ift_t first, ift_t last) {
    int total = (int) first.count + (int) last.count;
    if( total > IFTYPE_MAX_TAGS ) {
        sh_log_info( "IFT ERROR: IFTYPE_MAX_TAGS reached "
                "when combining types.");
        total = IFTYPE_MAX_TAGS;
    }
    ift_t result = first;
    int offset = result.count;
    result.count = (uint8_t) total;
    for(int i = 0; i < last.count; i++) {
        result.tags[i + offset] = last.tags[i];
    }
    return result;
}

ift_t ift_list(ift_t content_type) {
    ift_t type = { 0 };
    type.count = 1;
    type.tags[0] = IFT_LST;
    return ift_combine(type, content_type);
}

ift_t ift_extract(uint8_t* tags, int chunk_size) {
    assert(chunk_size < IFTYPE_MAX_TAGS && chunk_size > 0);
    if( chunk_size >= IFTYPE_MAX_TAGS || chunk_size < 0 )
        return ift_unknown();
    ift_t res = { 0 };
    res.count = (uint8_t) chunk_size;
    for(int i = 0; i < chunk_size; i++) {
        res.tags[i] = tags[i];
    }
    return res;
}

ift_t ift_list_get_content_type(ift_t list) {
    if( list.count < 2 )
        return ift_unknown();
    if( list.tags[0] != IFT_LST )
        return ift_unknown();
    return ift_extract(list.tags + 1, list.count - 1);
}

ift_t ift_append_tag(ift_t type, ift_tag_t tag) {
    if( type.count < (IFTYPE_MAX_TAGS - 1) ) {
        type.tags[type.count++] = tag;
    } else {
        sh_log_info( "IFT ERROR: IFTYPE_MAX_TAGS reached "
                "when combining types.");
    }
    return type;
}

ift_t ift_pop_last_tag(ift_t type) {
    if( type.count > 0 ) {
        type.count --;
    } else {
        sh_log_info("IFT ERROR: tried to remove from empty tag list.");
    }
    return type;
}

ift_t ift_func(ift_t return_type) {
    ift_t type = { 0 };
    type.count = 1;
    type.tags[0] = IFT_FUN;
    type = ift_combine(type, return_type);
    return ift_append_tag(type, IFT_ENDFUN);
}

ift_t ift_func_add_arg(ift_t func, ift_t arg) {
    assert(func.count > 0 && func.tags[func.count-1] == IFT_ENDFUN);
    func = ift_pop_last_tag(func);
    func = ift_combine(func, arg);
    return ift_append_tag(func, IFT_ENDFUN);
}

ift_t ift_func_1(ift_t return_type, ift_t arg1) {
    ift_t type = { 0 };
    type.count = 1;
    type.tags[0] = IFT_FUN;
    type = ift_combine(type, return_type);
    type = ift_combine(type, arg1);
    return ift_append_tag(type, IFT_ENDFUN);
}

ift_t ift_func_2(ift_t return_type, ift_t arg1, ift_t arg2) {
    ift_t type = { 0 };
    type.count = 1;
    type.tags[0] = IFT_FUN;
    type = ift_combine(type, return_type);
    type = ift_combine(type, arg1);
    type = ift_combine(type, arg2);
    return ift_append_tag(type, IFT_ENDFUN);
}

ift_t ift_func_3(ift_t return_type, ift_t arg1, ift_t arg2, ift_t arg3) {
    ift_t type = { 0 };
    type.count = 1;
    type.tags[0] = IFT_FUN;
    type = ift_combine(type, return_type);
    type = ift_combine(type, arg1);
    type = ift_combine(type, arg2);
    type = ift_combine(type, arg3);
    return ift_append_tag(type, IFT_ENDFUN);
}

int ift_get_flat_size(uint8_t* tags, int len) {
    if( len <= 0 )
        return 0;
    switch(tags[0]) {
        case IFT_LST: return 1 + ift_get_flat_size(tags + 1, len - 1);
        case IFT_FUN: {
            int fun_starts = 1;
            int index = 1;
            while(fun_starts > 0) {
                if( index >= len )
                    break;
                if( tags[index] == IFT_FUN )
                    fun_starts ++;
                else if( tags[index] == IFT_ENDFUN )
                    fun_starts --;
                index ++;
            }
            return index;
        }
        default: return 1;
    }
}

int ift_func_arg_count(ift_t func) {
    if( func.count == 0 )
        return 0;
    if( func.tags[0] != IFT_FUN )
        return 0;
    int i = 1 + ift_get_flat_size(func.tags + 1, (int) func.count - 1);
    int arg_count = 0;
    while(i < (func.count - 1)) { // count - 1 : because of ENDFUN
        arg_count ++;
        i += ift_get_flat_size(func.tags + i, (int) func.count - i);
    }
    return arg_count;
}

ift_t ift_func_get_arg(ift_t func, int index) {

    if( func.count == 0 )
        return ift_unknown();

    if( func.tags[0] != IFT_FUN )
        return ift_unknown();

    if( index < 0 )
        return ift_unknown();

    int i = 1 + ift_get_flat_size(func.tags + 1, (int) func.count - 1);
    int arg_index = 0;
    while(i < (func.count - 1)) { // count - 1 : because of ENDFUN
        int chunk_size = ift_get_flat_size(func.tags + i, (int) func.count - i);
        if( arg_index == index ) {
            return ift_extract(func.tags + i, chunk_size);
        }
        i += chunk_size;
        arg_index ++;
    }
    return ift_unknown();
}

ift_t ift_func_get_return_type(ift_t func) {
    if( func.count < 2 )
        return ift_unknown();
    if( func.tags[0] != IFT_FUN )
        return ift_unknown();
    int chunk_size = ift_get_flat_size(func.tags + 1, (int) func.count - 1);
    return ift_extract(func.tags + 1, chunk_size);
}

bool ift_type_equals(ift_t* a, ift_t* b) {
    if( a->count != b->count )
        return false;
    return memcmp(a->tags, b->tags, a->count) == 0;
}

sstr_t ift_type_to_sstr(ift_t type) {
    if( type.count == 0 )
        return sstr("<empty>");
    switch(type.tags[0]) {
        case IFT_UNK: return sstr("<unknown>");
        case IFT_VOID: return sstr("void");
        case IFT_BOOL: return sstr("bool");
        case IFT_CHAR: return sstr("char");
        case IFT_I32: return sstr("int");
        case IFT_F32: return sstr("float");
        case IFT_LST: {
            sstr_t s = sstr("array");
            sstr_t c = ift_type_to_sstr(ift_list_get_content_type(type));
            sstr_append_fmt(&s, "<%.*s>",
                sstr_len(&c), sstr_ptr(&c));
            return s;
        } break;
        case IFT_FUN: {
            sstr_t s = sstr("fun(");
            int nargs = ift_func_arg_count(type);
            for(int i = 0; i < nargs; i++) {
                if( i > 0 )
                    sstr_append_str(&s, ", ");
                sstr_t at = ift_type_to_sstr(ift_func_get_arg(type, i));
                sstr_append(&s, &at);
            }
            sstr_append_str(&s, ") -> ");
            sstr_t rt = ift_type_to_sstr(ift_func_get_return_type(type));
            sstr_append(&s, &rt);
            return s;
        }
    }
    return sstr("");
}

bool ift_is_unknown(ift_t type) {
    if( type.count == 0 )
        return true;
    return type.tags[0] == IFT_UNK;
}
