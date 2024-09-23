#include "gvm_asmutils.h"
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

uint32_t au_consts_add_number(valbuffer_t* consts, float value) {
    uint32_t existing = 0;
    if( valbuffer_find_float(consts, value, &existing) ) {
        return existing;
    }

    if( valbuffer_add(consts, val_number(value)) == false ) {
        printf("error: consts_add_number\n");
        return consts->size;
    }
    return consts->size - 1;
}

uint32_t au_consts_add_bool(valbuffer_t* consts, bool value) {
    uint32_t existing = 0;
    if( valbuffer_find_bool(consts, value, &existing) ) {
        return existing;
    }
    if(valbuffer_add(consts, val_bool(value)) == false) {
        printf("error: consts_add_bool\n");
        return consts->size;
    }
    return consts->size - 1;
}

uint32_t au_consts_add_char(valbuffer_t* consts, char value, bool force_contiguous) {
    if( force_contiguous == false ) {
        uint32_t existing = 0;
        if( valbuffer_find_char(consts, value, &existing) ) {
            return existing;
        }
    }
    if(valbuffer_add(consts, val_char(value)) == false) {
        printf("error: consts_add_char\n");
        return consts->size;
    }
    return consts->size - 1;
}

uint32_t au_consts_add_string(valbuffer_t* consts, char* text) {

    if( text[0] != '"' ) {
        printf("error: expected \" at start of string.\n");
    }

    text = text + 1;

    int in_len = string_count_until(text, '\"');
    
    // UN-ESCAPE input string
    // need this step (with malloc) to unescape '\n' etc.
    char* tmp_buffer = malloc((in_len + 1) * sizeof(char));
    int r_count = 0;
    int w_count = 0;
    while( r_count < in_len ) {
        if( text[r_count] == '\\' && (r_count + 1) < in_len ) {
            char next = text[r_count + 1];
            switch (next) {
                case 'n':
                    r_count += 2;
                    tmp_buffer[w_count++] = '\n';
                    continue; // continue next while-iteration
                case 't':
                    r_count += 2;
                    tmp_buffer[w_count++] = '\t';
                    continue; // continue next while-iteration
                case '\\':
                    r_count += 2;
                    tmp_buffer[w_count++] = '\\';
                    continue; // continue next while-iteration
                default:
                    printf("unhandled escaped character '\\%c'", next);
                    break;
            }
        }
        tmp_buffer[w_count++] = text[r_count++];
    }
    tmp_buffer[w_count] = '\0';
    
    uint32_t existing = 0;
    if( valbuffer_find_string(consts, tmp_buffer, w_count, &existing) ) {
        return existing;
    }

    int string_start = consts->size;
    for(int i = 0; i < w_count; i++) {
        au_consts_add_char(consts, tmp_buffer[i], true);
    }

    free(tmp_buffer); // free the UN-ESCAPE buffer

    bool ok = valbuffer_add(consts,
        val_array_from_args( MEM_MK_CONST_ADDR(string_start),
                  w_count));
    return ok
        ? (consts->size - 1)
        : (consts->size);
}

uint32_t au_consts_add_ivec2(valbuffer_t* consts, char* text) {

    if( text[0] != '(' ) {
        printf("error: expected ( at start of ivec2.\n");
    }

    text = text + 1; // skip open paren

    ivec2_t value = { 0 };

    // read x
    int to_comma = string_count_until(text, ',');
    value.x = string_parse_int(text, to_comma);

    text += to_comma + 1;
    
    // read y
    int to_rparen = string_count_until(text, ')');
    value.y = string_parse_int(text, to_rparen);

    uint32_t existing = 0;
    if( valbuffer_find_ivec2(consts, value, &existing) ) {
        return existing;
    }

    bool ok = valbuffer_add(consts, val_ivec2(value));
    return ok
        ? (consts->size - 1)
        : (consts->size);
}

uint32_t au_consts_add_symbol_as_string(valbuffer_t* consts, char* text, int length) {

    uint32_t existing = 0;
    if( valbuffer_find_string(consts, text, length, &existing)) {
        return existing;
    }

    int string_start = consts->size;
    for(int i = 0; i < length; i++) {
        au_consts_add_char(consts, text[i], true);
    }

    bool ok = valbuffer_add(consts,
        val_array_from_args( MEM_MK_CONST_ADDR(string_start),
                  length));
    return ok
        ? (consts->size - 1)
        : (consts->size);
}



