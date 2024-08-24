#include "gvm_asm.h"
#include "gvm_parser.h"
#include "gvm_value.h"
#include "gvm.h"
#include <string.h>

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

    char buf[128] = { 0 };
    parser_current_as_string(p, buf, 127);
    int nschemes = sizeof(schemes) / sizeof(schemes[0]);

    for(int i = 0; i < nschemes; i++) {
        if( strncmp(buf, schemes[i].name, 127) != 0 ) {
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

    printf(" [-]\n");
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

typedef struct const_section_t {
    int size;
    int capacity;
    val_t* constants;
} const_section_t;

gvm_result_t consts_init(const_section_t* consts, int capacity) {
    val_t* constants = (val_t*) malloc(capacity * sizeof(val_t));
    if( constants == NULL ) {
        return RES_OUT_OF_MEMORY;
    }
    consts->capacity = capacity;
    consts->constants = constants;
    consts->size = 0;
    return RES_OK;
}

gvm_result_t consts_add(const_section_t* consts, val_t value) {
    if( consts->size >= consts->capacity ) {
        int new_capacity = consts->size * 2;
        val_t* new_vals = (val_t*) realloc(consts->constants, new_capacity * sizeof(val_t));
        if( new_vals == NULL ) {
            return RES_OUT_OF_MEMORY;
        }
        consts->capacity = new_capacity;
        consts->constants = new_vals;
    }
    consts->constants[consts->size++] = value;
    return RES_OK;
}

int consts_add_number(const_section_t* consts, int16_t value) {
    gvm_result_t res = consts_add(consts, val_number(value));
    gvm_print_if_error(res, "consts_add");
    return consts->size - 1;
}

int consts_add_bool(const_section_t* consts, bool value) {
    gvm_result_t res = consts_add(consts, val_bool(value));
    gvm_print_if_error(res, "consts_add");
    return consts->size - 1;
}

int consts_add_char(const_section_t* consts, char value) {
    gvm_result_t res = consts_add(consts, val_char(value));
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

int consts_add_string(const_section_t* consts, char* text) {
    if( text[0] != '"' ) {
        printf("error: expected \" at start of string.\n");
    }
    text = text + 1;
    int string_length = string_count_until(text, '\"');
    val_t* start = consts->constants + consts->size;
    for(int i = 0; i < string_length; i++) {
        consts_add_char(consts, text[i]);
    }
    gvm_result_t res = consts_add(consts,
        val_list(start, (uint16_t)string_length));
    gvm_print_if_error(res, "consts_add_string");
    return consts->size - 1;
}

int asm_add_const(const_section_t* consts, parser_t* parser) {
    token_t token = parser_current(parser);
    switch (token.type)
    {
        case TT_STRING: {
            char* text = parser_get_token_char_ptr(parser, token);
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

gvm_result_t asm_assemble(char* code_buffer) {
    
    parser_t* parser = parser_create(code_buffer);
    if( parser == NULL ) {
        return RES_ERROR;
    }

    gvm_result_t result_code = RES_OK;
    label_set_t label_set = { 0 };

    const_section_t const_section = { 0 };
    consts_init(&const_section, 5);

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
            result_code = label_add(&label_set, label, length, address);
            if( result_code != RES_OK ) {
                goto on_error;
            }
            keep_going &= parser_consume(parser, TT_SYMBOL);
            keep_going &= parser_consume(parser, TT_COLON);
            keep_going &= parser_consume(parser, TT_SEPARATOR);
            continue;
        }

        // parse label / operatioon
        int scheme_index = scheme_match(parser);
        if( scheme_index >= 0 ) {
            op_scheme_t op_scheme = schemes[scheme_index];
            printf("op match: '%s'\n", op_scheme.name);
            keep_going &= parser_consume(parser, TT_SYMBOL);
            int nargs = scheme_get_arg_count(op_scheme.typespec);
            int arg_index = 0;
            while (arg_index < nargs && keep_going) {
                keep_going &= parser_consume(parser, TT_SEPARATOR);
                if( scheme_is_flag_set(op_scheme.isconst, arg_index) ) {
                    int const_index = asm_add_const(&const_section, parser);
                }
                keep_going &= parser_advance(parser);
                arg_index ++;
            }
            address ++;
        }

        // TODO
        // [ ]  read up constants and store them in table
        // [ ]  append to instruction list
        //      (find a way to replace labels with addresses)
    }

    // TODO
    // [ ]  walk over all the instructions and replace labels
    //      with addesses

    // debug print 
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


    // TODO: DESTROY CONSTANTS

on_error:
    parser_destroy(parser);
    return result_code;
}