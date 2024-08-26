#include "gvm_asm.h"
#include "gvm_parser.h"
#include "gvm_value.h"
#include "gvm.h"
#include "gvm_utils.h"
#include <string.h>
#include <assert.h>

typedef struct op_scheme_t {
    char name[64];
    gvm_op_t opcode;
    uint32_t typespec;
    uint32_t isconst;
    uint32_t islabel;
} op_scheme_t;

#define ARGSPEC1(A1) (uint64_t)((uint8_t)((A1)))
#define ARGSPEC2(A1, A2) (ARGSPEC1(A1) | (uint64_t)((uint8_t)((A2)) << 4))

static op_scheme_t schemes[] = {
//  [string name]  [op bytecode id]    [arg type]              [store as const]   [is label reference]
    {"push",        OP_PUSH,            ARGSPEC1(TT_NUMBER),    0x01,              0x00 },
    {"push",        OP_PUSH,            ARGSPEC1(TT_STRING),    0x01,              0x00 },
    {"dup",         OP_DUP,             ARGSPEC1(TT_NUMBER),    0x00,              0x00 },
    {"is-less",     OP_CMP_LESS_THAN,   ARGSPEC1(0),            0x00,              0x00 },
    {"if-false",    OP_JUMP_IF_FALSE,   ARGSPEC1(TT_SYMBOL),    0x00,              0x01 },
    {"jump",        OP_JUMP,            ARGSPEC1(TT_SYMBOL),    0x00,              0x01 },
    {"add",         OP_ADD,             ARGSPEC1(0),            0x00,              0x00 },
    {"exit",        OP_EXIT,            ARGSPEC1(TT_NUMBER),    0x00,              0x00 }
};

int scheme_get_arg_count(uint32_t typespec) {
    int arg_count = 0;
    while (typespec > 0) {
        typespec = (typespec >> 4);
        arg_count ++;
    }
    return arg_count;
}

bool scheme_is_flag_set(uint32_t bit4flags, int arg_index) {
    return (0xF & (bit4flags >> arg_index)) > 0;
}

int scheme_match(parser_t* p) {

    token_t token = parser_current(p);
    char* str = parser_get_token_string_ptr(p, token);
    int str_len = parser_get_token_string_length(p, token);
    int nschemes = sizeof(schemes) / sizeof(schemes[0]);

    for(int i = 0; i < nschemes; i++) {
        if( strncmp(str, schemes[i].name, str_len) != 0 ) {
            continue;
        }
        uint32_t typespec = schemes[i].typespec;
        int lookahead = 1; 
        bool match = true;
        while (typespec > 0) {
            if( parser_peek(p, lookahead).type != TT_SEPARATOR ) {
                match = false;
                break;
            }
            lookahead ++;
            if( (token_type_t)(0xF & typespec) != parser_peek(p, lookahead).type ) {
                match = false;
                break;
            }
            lookahead ++;
            typespec = (typespec >> 4);
        }
        if( match ) {
            return i;
        }
    }

    return -1;
}

#define MAX_LABELS 64

typedef struct label_set_t {
    char* label[MAX_LABELS];
    int length[MAX_LABELS];
    int address[MAX_LABELS];
    int count;
} label_set_t;

gvm_result_t label_add(label_set_t* set, char* str, int len, int address) {
    if( set->count > MAX_LABELS ) {
        printf("error: hit max labels threashold\n");
        return RES_NOT_SUPPORTED;
    }
    set->label[set->count] = str;
    set->length[set->count] = len;
    set->address[set->count] = address;
    set->count ++;
    return RES_OK;
}

int label_get_address(label_set_t* set, char* str, int len) {
    for(int i = 0; i < set->count; i++) {
        if( strncmp(set->label[i], str, len) == 0 ) {
            return set->address[i];
        }
    }
    return -1;
}

int consts_add_number(val_buffer_t* consts, int16_t value) {
    gvm_result_t res = val_buffer_add_update_refs(consts, val_number(value));
    gvm_print_if_error(res, "consts_add");
    return consts->size - 1;
}

int consts_add_bool(val_buffer_t* consts, bool value) {
    gvm_result_t res = val_buffer_add_update_refs(consts, val_bool(value));
    gvm_print_if_error(res, "consts_add");
    return consts->size - 1;
}

int consts_add_char(val_buffer_t* consts, char value) {
    gvm_result_t res = val_buffer_add_update_refs(consts, val_char(value));
    gvm_print_if_error(res, "consts_add");
    return consts->size - 1;
}

int string_count_until(char* text, char stopchar) {
    int len = strlen(text);
    for(int i = 0; i < len; i++) {
        if( text[i] == stopchar ) {
            return i;
        }
    }
    return len;
}

int consts_add_string(val_buffer_t* consts, char* text) {
    if( text[0] != '"' ) {
        printf("error: expected \" at start of string.\n");
    }
    text = text + 1;
    int string_length = string_count_until(text, '\"');
    for(int i = 0; i < string_length; i++) {
        consts_add_char(consts, text[i]);
    }
    val_t* start = &consts->values[consts->size - string_length];
    gvm_result_t res = val_buffer_add_update_refs(consts,
        val_list(start, (uint16_t)string_length));
    gvm_print_if_error(res, "consts_add_string");
    return consts->size - 1;
}

int consts_add_current(val_buffer_t* consts, parser_t* parser) {
    token_t token = parser_current(parser);
    switch (token.type)
    {
        case TT_STRING: {
            char* text = parser_get_token_string_ptr(parser, token);
            return consts_add_string(consts, text);
        }
        case TT_NUMBER: {
            int value = parser_get_token_int_value(parser, token);
            return consts_add_number(consts, (int16_t) value);
        }
        default: {
            printf("cant load %s as const.\n", parser_tt_to_str(token.type));
            return consts->size - 1;
        }
    }
}


#define DBG_LOG(...) printf(__VA_ARGS__)
#define DBG_LOG_CONST(C, I) val_print(&(C)->values[(I)])

gvm_result_t asm_scan_labels(parser_t* parser, label_set_t* label_set) {
    bool keep_going = true;
    int address = 0;

    while ( parser_is_at_end(parser) == false && keep_going ) {
        // consume separators
        while (parser_match(parser, TT_SEPARATOR)) {
            keep_going &= parser_consume(parser, TT_SEPARATOR);
        }

        token_t current = parser_current(parser);
        token_t next = parser_peek(parser, 1);

        if( current.type == TT_SYMBOL && next.type == TT_COLON ) {
            char* label = parser->text.array + current.src_index;
            int length = next.src_index - current.src_index;
            DBG_LOG("> add label: ");
            DBG_LOG("%.*s", length, label);
            DBG_LOG(" (%i)\n", address);
            gvm_result_t res = label_add(label_set, label, length, address);
            if( res != RES_OK ) {
                return res;
            }
            keep_going &= parser_consume(parser, TT_SYMBOL);
            keep_going &= parser_consume(parser, TT_COLON);
            keep_going &= parser_consume(parser, TT_SEPARATOR);
            continue;
        }

        int scheme_index = scheme_match(parser);
        if( scheme_index >= 0 ) {
            op_scheme_t op_scheme = schemes[scheme_index];
            keep_going &= parser_consume(parser, TT_SYMBOL);
            int nargs = scheme_get_arg_count(op_scheme.typespec);
            while (nargs-- > 0) {
                keep_going &= parser_consume(parser, TT_SEPARATOR);
                keep_going &= parser_advance(parser);
            }
            // op handled; next instruction
            address ++;
            keep_going &= parser_advance(parser);
        } else {
            token_t token = parser_current(parser);
            char* str = parser_get_token_string_ptr(parser, token);
            int str_len = parser_get_token_string_length(parser, token);
            str_len = str_len < 128 ? str_len : 127;
            char buf[128] = { 0 };
            strncpy(buf, str, str_len);
            printf("no operation matching '%s'.\n", buf);
            return RES_NOT_SUPPORTED;
        }
    }

    return RES_OK;
}

gvm_result_t asm_assemble(char* code_buffer) {
    
    parser_t* parser = parser_create(code_buffer);
    if( parser == NULL ) {
        return RES_ERROR;
    }

    gvm_result_t result_code = RES_OK;
    label_set_t label_set = { 0 };

    result_code = asm_scan_labels(parser, &label_set);
    if( result_code != RES_OK ) {
        goto on_error;
    }

    parser_reset(parser);

    // todo: handle results

    val_buffer_t const_store = { 0 };
    val_buffer_create(&const_store, 5);

    u8buffer_t code_section = { 0 };
    u8buffer_create(&code_section, 16);

    bool keep_going = true;

    while ( parser_is_at_end(parser) == false && keep_going ) {

        // consume separators
        while (parser_match(parser, TT_SEPARATOR)) {
            keep_going &= parser_consume(parser, TT_SEPARATOR);
        }

        // skip label definitions this time
        if( parser_current(parser).type == TT_SYMBOL
            && parser_peek(parser, 1).type == TT_COLON )
        {
            keep_going &= parser_consume(parser, TT_SYMBOL);
            keep_going &= parser_consume(parser, TT_COLON);
            keep_going &= parser_consume(parser, TT_SEPARATOR);
            continue;
        }

        // parse operatioon
        int scheme_index = scheme_match(parser);
        if( scheme_index >= 0 ) {
            op_scheme_t op_scheme = schemes[scheme_index];
            DBG_LOG("> op: ");
            DBG_LOG("'%s' ", op_scheme.name);
            u8buffer_write(&code_section, (uint8_t) op_scheme.opcode);
            keep_going &= parser_consume(parser, TT_SYMBOL);
            int nargs = scheme_get_arg_count(op_scheme.typespec);
            int arg_index = 0;
            while (arg_index < nargs && keep_going) {
                keep_going &= parser_consume(parser, TT_SEPARATOR);
                if( scheme_is_flag_set(op_scheme.isconst, arg_index) ) {
                    int const_index = consts_add_current(&const_store, parser);
                    DBG_LOG("%i (", const_index);
                    DBG_LOG_CONST(&const_store, const_index);
                    DBG_LOG(")");
                    assert(const_index >= 0 && const_index < 256);
                    u8buffer_write(&code_section, (uint8_t) const_index);
                } else if (scheme_is_flag_set(op_scheme.islabel, arg_index)) {
                    token_t token = parser_current(parser);
                    int len = parser_get_token_string_length(parser, token);
                    char* ptr = parser_get_token_string_ptr(parser, token);
                    int label_index = label_get_address(&label_set, ptr, len);
                    DBG_LOG("%i (label) ", label_index);
                    assert(label_index >= 0 && label_index < 256);
                    u8buffer_write(&code_section, (uint8_t) label_index);
                } else {
                    DBG_LOG("(other) ");
                }
                keep_going &= parser_advance(parser);
                arg_index ++;
            }
            DBG_LOG("\n");
        }
    }


    val_buffer_print(&const_store);

    // debug print
#ifdef DGB_PRINT
    printf("[LABELS]\n");
    for(int i = 0; i < label_set.count; i++) {
        char buf[128] = { 0 };
        snprintf(buf, label_set.length[i] + 1, label_set.label[i], "%s");
        printf("  LABEL: '%s' (%i)\n", buf, label_set.address[i]);
    }

    printf("[CONSTANTS]\n");
    for(int i = 0; i < const_section.size; i++) {
        printf("  ");
        val_print(&const_section.constants[i]);
        printf("\n");
    }
#endif


    // TODO: DESTROY CONSTANTS TABLE

on_error:
    parser_destroy(parser);
    return result_code;
}