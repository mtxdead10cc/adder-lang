#include "gvm_asm.h"
#include "gvm_parser.h"
#include <string.h>

typedef struct op_scheme_t {
    char name[64];
    gvm_op_t opcode;
    uint64_t argspec;
    int argcount;
} op_scheme_t;


#define ARGSPEC1(A1) (uint64_t)((uint8_t)((A1)))
#define ARGSPEC2(A1, A2) (ARGSPEC1(A1) | (uint64_t)((uint8_t)((A2)) << 4))

static op_scheme_t schemes[] = {
    {"push",        OP_PUSH,            ARGSPEC1(TT_NUMBER),    1},
    {"push",        OP_PUSH,            ARGSPEC1(TT_STRING),    1},
    {"dup",         OP_DUP,             ARGSPEC1(TT_NUMBER),    1},
    {"is-less",     OP_CMP_LESS_THAN,   0,                      0},
    {"if-false",    OP_JUMP_IF_FALSE,   ARGSPEC1(TT_SYMBOL),    1},
    {"jump",        OP_JUMP,            ARGSPEC1(TT_SYMBOL),    1},
    {"add",         OP_ADD,             0,                      0},
    {"exit",        OP_EXIT,            ARGSPEC1(TT_NUMBER),    1}
};

int match_scheme(parser_t* p) {

    char buf[128] = { 0 };
    parser_current_as_string(p, buf, 127);
    int nschemes = sizeof(schemes) / sizeof(schemes[0]);

    for(int i = 0; i < nschemes; i++) {
        if( strncmp(buf, schemes[i].name, 127) != 0 ) {
            continue;
        }
        uint64_t argspec = schemes[i].argspec;
        int lookahead = 1; 
        bool match = true;
        while (argspec > 0) {
            if( parser_peek(p, lookahead).type != TT_SEPARATOR ) {
                match = false;
                break;
            }
            lookahead ++;
            if( (token_type_t)(0xF & argspec) != parser_peek(p, lookahead).type ) {
                match = false;
                break;
            }
            lookahead ++;
            argspec = (argspec >> 4);
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

gvm_result_t asm_assemble(char* code_buffer) {
    
    parser_t* parser = parser_create(code_buffer);
    if( parser == NULL ) {
        return RES_ERROR;
    }

    gvm_result_t result_code = RES_OK;
    label_set_t label_set = { 0 };

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
        int scheme_index = match_scheme(parser);
        if( scheme_index >= 0 ) {
            printf("op match: '%s'\n", schemes[scheme_index].name);
            keep_going &= parser_consume(parser, TT_SYMBOL);
            int nargs = schemes[scheme_index].argcount;
            while (nargs > 0 && keep_going) {
                keep_going &= parser_consume(parser, TT_SEPARATOR);
                keep_going &= parser_advance(parser);
                nargs -= 1;
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
    for(int i = 0; i < label_set.count; i++) {
        char buf[128] = { 0 };
        snprintf(buf, label_set.length[i] + 1, label_set.label[i], "%s");
        printf("LABEL: '%s' (%i)\n", buf, label_set.address[i]);
    }

on_error:
    parser_destroy(parser);
    return result_code;
}