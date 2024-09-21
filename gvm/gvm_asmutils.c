#include "gvm_asmutils.h"
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

char* au_get_op_name(gvm_op_t opcode) {
    assert(OP_OPCODE_COUNT == 32 && "Opcode count changed.");
    switch(opcode) {
        case OP_HALT:               return "OP_HALT";
        case OP_AND:                return "OP_AND";
        case OP_OR:                 return "OP_OR";
        case OP_NOT:                return "OP_NOT";
        case OP_MUL:                return "OP_MUL";
        case OP_ADD:                return "OP_ADD";
        case OP_SUB:                return "OP_SUB";
        case OP_NEG:                return "OP_NEG";
        case OP_DUP_1:              return "OP_DUP_1";
        case OP_DUP_2:              return "OP_DUP_2";
        case OP_ROT_2:              return "OP_ROT_2";
        case OP_CMP_EQUAL:          return "OP_CMP_EQUAL";
        case OP_CMP_LESS_THAN:      return "OP_CMP_LESS_THAN";
        case OP_CMP_MORE_THAN:      return "OP_CMP_MORE_THAN";
        case OP_PUSH_VALUE:         return "OP_PUSH_VALUE";
        case OP_POP_1:              return "OP_POP_1";
        case OP_POP_2:              return "OP_POP_2";
        case OP_JUMP:               return "OP_JUMP";
        case OP_JUMP_IF_FALSE:      return "OP_JUMP_IF_FALSE";
        case OP_EXIT:               return "OP_EXIT";
        case OP_CALL:               return "OP_CALL";
        case OP_INIT:               return "OP_INIT";
        case OP_MAKE_FRAME:         return "OP_MAKE_FRAME";
        case OP_RETURN:             return "OP_RETURN";
        case OP_STORE_LOCAL:        return "OP_STORE_LOCAL";
        case OP_LOAD_LOCAL:         return "OP_LOAD_LOCAL";
        case OP_PRINT:              return "OP_PRINT";
        case OP_MAKE_ARRAY:         return "OP_MAKE_ARRAY";
        case OP_ARRAY_LENGTH:       return "OP_ARRAY_LENGTH";
        case OP_MAKE_ITER:          return "OP_MAKE_ITER";
        case OP_ITER_NEXT:          return "OP_ITER_NEXT";
        case OP_CALL_NATIVE:        return "OP_CALL_NATIVE";
        default:                    return "<OP-UNKNOWN>";
    }
}

int au_get_op_instr_arg_count(gvm_op_t opcode) {
    assert(OP_OPCODE_COUNT == 31 && "Opcode count changed.");
    switch(opcode) {
        case OP_HALT:
        case OP_AND:
        case OP_OR:
        case OP_NOT:
        case OP_MUL:
        case OP_ADD:
        case OP_SUB:
        case OP_NEG:
        case OP_DUP_1:
        case OP_DUP_2:
        case OP_ROT_2:
        case OP_CMP_EQUAL:
        case OP_CMP_LESS_THAN:
        case OP_POP_1:
        case OP_POP_2:
        case OP_RETURN:
        case OP_PRINT:
        case OP_MAKE_ARRAY:
        case OP_CMP_MORE_THAN:
            return 0;
        case OP_PUSH_VALUE:   
        case OP_JUMP:         
        case OP_JUMP_IF_FALSE:
        case OP_EXIT:
        case OP_CALL:
        case OP_INIT:
        case OP_STORE_LOCAL:
        case OP_LOAD_LOCAL:
        case OP_ARRAY_LENGTH:
        case OP_MAKE_ITER:
        case OP_ITER_NEXT:
        case OP_CALL_NATIVE:
            return 1;
        case OP_MAKE_FRAME:
            return 2;
        default:
            return -1;
    }
}

void au_write_instruction(u8buffer_t* buf, int count, ...) {
    va_list ap;
    va_start(ap, count);

    int opcode = va_arg(ap, int);
    count -= 1; // remove the op code from the arg count
    
    COMPILER_ASSERT_MSG(opcode > 0 && opcode < OP_OPCODE_COUNT,
        "invalid op code %i",
        opcode);

    COMPILER_ASSERT_MSG(u8buffer_write(buf, opcode),
        "Failed to allocate memory for instruction, aborting.");

    int expected_arg_count = au_get_op_instr_arg_count(opcode);
    COMPILER_ASSERT_MSG(count == expected_arg_count, 
        "Invalid instruction argument count for '%s' (%i)\n"
        "  expected %i\n"
        "  received %i",
        au_get_op_name(opcode), opcode, expected_arg_count, count);

    for(int i = 0; i < count; i++) {
        int arg = va_arg(ap, int);
        COMPILER_ASSERT_MSG(u8buffer_write(buf, (uint8_t)(arg & 0xFF)),
            "failed to allocate memory for instruction, aborting.");
        COMPILER_ASSERT_MSG(u8buffer_write(buf, (uint8_t)((arg >> 8) & 0xFF)),
            "failed to allocate memory for instruction, aborting.");
    }
    va_end(ap);
}

int au_consts_add_number(valbuffer_t* consts, float value) {
    int existing = valbuffer_find_float(consts, value);
    if( existing >= 0 ) {
        return existing;
    }

    if( valbuffer_add(consts, val_number(value)) == false ) {
        printf("error: consts_add_number\n");
        return consts->size;
    }
    return consts->size - 1;
}

int au_consts_add_bool(valbuffer_t* consts, bool value) {
    int existing = valbuffer_find_bool(consts, value);
    if( existing >= 0 ) {
        return existing;
    }
    if(valbuffer_add(consts, val_bool(value)) == false) {
        printf("error: consts_add_bool\n");
        return consts->size;
    }
    return consts->size - 1;
}

int au_consts_add_char(valbuffer_t* consts, char value, bool force_contiguous) {
    if( force_contiguous == false ) {
        int existing = valbuffer_find_char(consts, value);
        if( existing >= 0 ) {
            return existing;
        }
    }
    if(valbuffer_add(consts, val_char(value)) == false) {
        printf("error: consts_add_char\n");
        return consts->size;
    }
    return consts->size - 1;
}

int au_consts_add_string(valbuffer_t* consts, char* text) {

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
    
    int existing = valbuffer_find_string(consts, tmp_buffer, w_count);
    if( existing >= 0 ) {
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

int au_consts_add_ivec2(valbuffer_t* consts, char* text) {

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

    int existing = valbuffer_find_ivec2(consts, value);
    if( existing >= 0 ) {
        return existing;
    }

    bool ok = valbuffer_add(consts, val_ivec2(value));
    return ok
        ? (consts->size - 1)
        : (consts->size);
}

int au_consts_add_symbol_as_string(valbuffer_t* consts, char* text, int length) {

    int existing = valbuffer_find_string(consts, text, length);
    if( existing >= 0 ) {
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



