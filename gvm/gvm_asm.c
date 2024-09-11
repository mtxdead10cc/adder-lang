#include "gvm_asm.h"
#include "gvm_parser.h"
#include "gvm_value.h"
#include "gvm.h"
#include "gvm_utils.h"
#include "gvm_memory.h"
#include "gvm_config.h"
#include <string.h>
#include <assert.h>

typedef struct op_scheme_t {
    char name[64];
    gvm_op_t opcode;
    uint32_t typespec;
    uint32_t isconst;
    uint32_t islabel;
    uint32_t isreg;
} op_scheme_t;

#define ARGSPEC1(A1) (uint64_t)((uint8_t)((A1)))
#define ARGSPEC2(A1, A2) (ARGSPEC1(A1) | (uint64_t)((uint8_t)((A2)) << 4))

static op_scheme_t schemes[] = {
//  [string name]      [op bytecode id]    [arg type]                       [store as const]   [label reference]    [register reference]
    {"push",            OP_PUSH_VALUE,      ARGSPEC1(TT_STRING),             0x01,              0x00,                0x00 },
    {"push",            OP_PUSH_VALUE,      ARGSPEC1(TT_NUMBER),             0x01,              0x00,                0x00 },
    {"push",            OP_PUSH_VALUE,      ARGSPEC1(TT_VEC2),               0x01,              0x00,                0x00 },
    {"globstore",       OP_STORE_GLOBAL,    ARGSPEC1(TT_SYMBOL),             0x00,              0x00,                0x01 },
    {"globload",        OP_LOAD_GLOBAL,     ARGSPEC1(TT_SYMBOL),             0x00,              0x00,                0x01 },
    {"store",           OP_STORE_LOCAL,     ARGSPEC1(TT_NUMBER),             0x00,              0x00,                0x00 },
    {"load",            OP_LOAD_LOCAL,      ARGSPEC1(TT_NUMBER),             0x00,              0x00,                0x00 },
    {"print",           OP_PRINT,           ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"call",            OP_CALL,            ARGSPEC1(TT_SYMBOL),             0x00,              0x01,                0x00 },
    {"frame",           OP_MAKE_FRAME,      ARGSPEC2(TT_NUMBER, TT_NUMBER),  0x00,              0x00,                0x00 },
    {"array",           OP_MAKE_ARRAY,      ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"array-len",       OP_ARRAY_LENGTH,    ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"iter",            OP_MAKE_ITER,       ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"iter-next",       OP_ITER_NEXT,       ARGSPEC1(TT_SYMBOL),             0x00,              0x01,                0x00 },
    {"return",          OP_RETURN,          ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"pop1",            OP_POP_1,           ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"pop2",            OP_POP_2,           ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"dup1",            OP_DUP_1,           ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"dup2",            OP_DUP_2,           ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"rot2",            OP_ROT_2,           ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"is-less",         OP_CMP_LESS_THAN,   ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"is-more",         OP_CMP_MORE_THAN,   ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"is-equal",        OP_CMP_EQUAL,       ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"if-false",        OP_JUMP_IF_FALSE,   ARGSPEC1(TT_SYMBOL),             0x00,              0x01,                0x00 },
    {"jump",            OP_JUMP,            ARGSPEC1(TT_SYMBOL),             0x00,              0x01,                0x00 },
    {"exit",            OP_EXIT,            ARGSPEC1(TT_NUMBER),             0x00,              0x00,                0x00 },
    {"and",             OP_AND,             ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"or",              OP_OR,              ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"nor",             OP_NOR,             ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"mul",             OP_MUL,             ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"add",             OP_ADD,             ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"sub",             OP_SUB,             ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"neg",             OP_NEG,             ARGSPEC1(0),                     0x00,              0x00,                0x00 },
    {"ncall",           OP_CALL_NATIVE,     ARGSPEC1(TT_SYMBOL),             0x01,              0x00,                0x00 }
};

#if GVM_TRACE_LOG_LEVEL >= 3
static inline void asm_debug_print_token(parser_t* parser) {
    token_t token = parser_current(parser);
    char* str = parser_get_token_string_ptr(parser, token);
    int str_len = parser_get_token_string_length(parser, token);
    printf("%.*s", str_len, str);
}
# define DBG_LOG(...) printf(__VA_ARGS__)
# define DBG_LOG_CONST(C, I) val_print_lookup_val_array((C).values, (C).values[(I)])
# define DBG_LOG_OPERAND(P) asm_debug_print_token(P)
#else
# define DBG_LOG(...)
# define DBG_LOG_CONST(C, I)
# define DBG_LOG_OPERAND(P)
#endif

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
        if( strlen(schemes[i].name) != (size_t) str_len ) {
            continue;
        }
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
            token_type_t expected = (token_type_t)(0xF & typespec);
            token_type_t actual = parser_peek(p, lookahead).type;
            if( expected != actual ) {
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

int consts_add_number(valbuffer_t* consts, float value) {
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

int consts_add_bool(valbuffer_t* consts, bool value) {
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

int consts_add_char(valbuffer_t* consts, char value, bool force_contiguous) {
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

int consts_add_string(valbuffer_t* consts, char* text) {

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
        consts_add_char(consts, tmp_buffer[i], true);
    }

    free(tmp_buffer); // free the UN-ESCAPE buffer

    bool ok = valbuffer_add(consts,
        val_array_from_args( MEM_MK_CONST_ADDR(string_start),
                  w_count));
    return ok
        ? (consts->size - 1)
        : (consts->size);
}

int consts_add_ivec2(valbuffer_t* consts, char* text) {

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

int consts_add_symbol_as_string(valbuffer_t* consts, char* text, int length) {

    int existing = valbuffer_find_string(consts, text, length);
    if( existing >= 0 ) {
        return existing;
    }

    int string_start = consts->size;
    for(int i = 0; i < length; i++) {
        consts_add_char(consts, text[i], true);
    }

    bool ok = valbuffer_add(consts,
        val_array_from_args( MEM_MK_CONST_ADDR(string_start),
                  length));
    return ok
        ? (consts->size - 1)
        : (consts->size);
}

/* [consts_add_current]
    Adds the current token to the constants buffer.
    TT_STRING -> VAL_ARRAY
    TT_NUMBER -> VAL_NUMBER
    TT_SYMBOL -> VAL_BOOL | VAL_STRING */
int consts_add_current(valbuffer_t* consts, parser_t* parser) {
    
    token_t token = parser_current(parser);
    
    char* text = parser_get_token_string_ptr(parser, token);
    int text_len = parser_get_token_string_length(parser, token);

    int const_index = consts->size - 1;
    switch (token.type)
    {
        case TT_STRING: {
            const_index = consts_add_string(consts, text);
        } break;
        case TT_NUMBER: {
            float value = parser_get_token_float_value(parser, token);
            const_index = consts_add_number(consts, value);
        } break;
        case TT_VEC2: {
            const_index = consts_add_ivec2(consts, text);
        } break;
        case TT_SYMBOL: {
            bool is_bool_true = false;
            bool is_bool_false = false;
            if( text_len <= 5 && text_len >= 4 ) {
                is_bool_true = strncmp(text, "true", 4) == 0;
                is_bool_false = strncmp(text, "false", 5) == 0;
            }
            if( is_bool_false ) {
                const_index = consts_add_bool(consts, false);
            } else if ( is_bool_true ) {
                const_index = consts_add_bool(consts, true);
            } else {
                const_index = consts_add_symbol_as_string(consts, text, text_len);
            }
        } break;
        default: {
            printf("cant load %s as const.\n", parser_tt_to_str(token.type));
            const_index = consts->size - 1;
        } break;
    }

    return const_index;
}

typedef struct label_set_t {
    char* label[GVM_ASM_MAX_LABELS];
    int length[GVM_ASM_MAX_LABELS];
    int address[GVM_ASM_MAX_LABELS];
    int count;
} label_set_t;

gvm_result_t label_add(label_set_t* set, char* str, int len, int address) {

    if( set->count > GVM_ASM_MAX_LABELS ) {
        printf("error: hit max labels threashold\n");
        return RES_NOT_SUPPORTED;
    }

    // check that the label name is not a reserved name 
    int nschemes = sizeof(schemes) / sizeof(schemes[0]);
    for(int i = 0; i < nschemes; i++) {
        if(strncmp(schemes[i].name, str, len) == 0) {
            printf("error: invalid label name, '%.*s' is a reserved keyword.\n", len, str);
            return RES_INVALID_INPUT;
        }
    }

    for(int i = 0; i < set->count; i++) {
        char* existing = set->label[i];
        int len = set->length[i];
        // already added
        if( strncmp(existing, str, len) == 0 ) {
            printf("error: duplicate label definition '%.*s'.\n", len, existing);
            return RES_INVALID_INPUT;
        }
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

typedef struct register_set_t {
    char* label[GVM_ASM_MAX_REGISTERS];
    int length[GVM_ASM_MAX_REGISTERS];
    int count;
} register_set_t;

int reg_find_index(register_set_t* set, char* str, int len) {
    for(int i = 0; i < set->count; i++) {
        if( strncmp(set->label[i], str, len) == 0 ) {
            return i;
        }
    }
    return -1;
}

int reg_add(register_set_t* set, char* str, int len) {
    if( set->count > GVM_ASM_MAX_REGISTERS ) {
        printf("error: hit max registers threashold\n");
        return -1;
    }
    int index = reg_find_index(set, str, len);
    if( index >= 0 ) {
        return index;
    }
    set->label[set->count] = str;
    set->length[set->count] = len;
    set->count ++;
    return set->count - 1;
}

/* [asm_scan_labels]
    Runs through all the tokens counting bytes in order to 
    keep track of op addresses. For every label encountered
    the label name and address (byte index) is stored in
    the provided label_set. */
gvm_result_t asm_scan_labels(parser_t* parser, label_set_t* label_set) {

    bool keep_going = true;
    int address = 0;

    while ( parser_is_at_end(parser) == false && keep_going ) {

        // consume comments & separators
        while ( parser_match(parser, TT_SEPARATOR) || parser_match(parser, TT_COMMENT) ) {
            parser_advance(parser);
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

        if( current.type == TT_COMMENT || current.type == TT_SEPARATOR ){
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
                address += 2; // 16 bit integer args
            }
            // op handled; next instruction
            address += 1;
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

void u8buffer_write_i16(u8buffer_t* buffer, int16_t val) {
    u8buffer_write(buffer, 0xFF & val);
    u8buffer_write(buffer, 0xFF & (val >> 8));
}

/* [asm_assemble_code_object]
*   Creates an internat parser_t, scans all the labels into a
*   label_set and then;
*   - writes the opcode of each instruction to the byte code
*     after converting op-name to opcode.
*   - op arg labels ar converted into byte code index (address)
*     by lookup through the label set.
*   - constants are read from tokens and stored as val_t in a 
*     valbuffer_t. The constant is referred to by its index (in
*     the val_buffer) in the byte code that is generated.
*/
byte_code_block_t asm_assemble_code_object(char* code_buffer) {
    
    parser_t* parser = parser_create(code_buffer);
    if( parser == NULL ) {
        return (byte_code_block_t) { 0 };
    }

    gvm_result_t result_code = RES_OK;
    label_set_t label_set = { 0 };
    register_set_t reg_set = { 0 };
    u8buffer_t code_section = { 0 };
    valbuffer_t const_store = { 0 };

#if GVM_TRACE_LOG_LEVEL >= 4
    printf("START TOKENS\n");
    parser_debug_print_tokens(parser);
    printf("END TOKENS\n");
#endif

    result_code = asm_scan_labels(parser, &label_set);
    if( result_code != RES_OK ) {
        goto on_error;
    }

    parser_reset(parser);

    if( valbuffer_create(&const_store, 5) == false ) {
        result_code = RES_OUT_OF_MEMORY;
        goto on_error;
    }
    
    if( u8buffer_create(&code_section, 16) == false ) {
        result_code = RES_OUT_OF_MEMORY;
        goto on_error;
    }

    bool keep_going = true;

    while ( parser_is_at_end(parser) == false && keep_going ) {

        // consume comments & separators
        while ( parser_match(parser, TT_SEPARATOR) || parser_match(parser, TT_COMMENT) ) {
            parser_advance(parser);
        }

        // skip label definitions this time
        token_t current = parser_current(parser);
        token_t next = parser_peek(parser, 1);
        if( current.type == TT_SYMBOL && next.type == TT_COLON ) {
            keep_going &= parser_consume(parser, TT_SYMBOL);
            keep_going &= parser_consume(parser, TT_COLON);
            keep_going &= parser_consume(parser, TT_SEPARATOR);
            continue;
        }

        if( current.type == TT_COMMENT || current.type == TT_SEPARATOR ) {
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
                    DBG_LOG_CONST(const_store, const_index);
                    DBG_LOG(")");
                    assert(const_index >= 0 && const_index < 256);
                    u8buffer_write_i16(&code_section, const_index);
                } else if (scheme_is_flag_set(op_scheme.islabel, arg_index)) {
                    token_t token = parser_current(parser);
                    int len = parser_get_token_string_length(parser, token);
                    char* ptr = parser_get_token_string_ptr(parser, token);
                    int label_index = label_get_address(&label_set, ptr, len);
                    if( label_index < 0 ) {
                        printf("error: label '%.*s' not found.\n", len, ptr);
                        result_code = RES_NOT_SUPPORTED;
                        goto on_error;
                    }
                    DBG_LOG("%i (%.*s) ", label_index, len, ptr);
                    assert(label_index >= 0 && label_index < 256);
                    u8buffer_write_i16(&code_section, label_index);
                } else if (scheme_is_flag_set(op_scheme.isreg, arg_index)) {
                    token_t token = parser_current(parser);
                    int len = parser_get_token_string_length(parser, token);
                    char* ptr = parser_get_token_string_ptr(parser, token);
                    int reg_index = reg_add(&reg_set, ptr, len);
                    if( reg_index < 0 ) {
                        printf("error: global '%.*s' not found.\n", len, ptr);
                        result_code = RES_NOT_SUPPORTED;
                        goto on_error;
                    }
                    DBG_LOG("%i (%.*s) ", reg_index, len, ptr);
                    assert(reg_index >= 0 && reg_index < 256);
                    u8buffer_write_i16(&code_section, reg_index);
                } else {
                    DBG_LOG_OPERAND(parser);
                    DBG_LOG(" ");
                    token_t token = parser_current(parser);
                    int operand = parser_get_token_float_value(parser, token);
                    u8buffer_write_i16(&code_section, operand);
                }
                keep_going &= parser_advance(parser);
                arg_index ++;
            }
            DBG_LOG("\n");
        }

    }

#if GVM_TRACE_LOG_LEVEL >= 3
    printf("START CONSTANTS\n");
    for(int i = 0; i < const_store.size; i++) {
        printf("  %3i: ", i);
        val_print_lookup_val_array(
            const_store.values,
            const_store.values[i]);
        printf("\n");
    }
    printf("END CONSTANTS\n");
#endif

    byte_code_block_t obj = { 0 };
    int byte_count_consts = sizeof(val_t) * const_store.size;
    int byte_count_code = code_section.size;
    int byte_count_header = 4;

    obj.size = byte_count_consts + byte_count_code + byte_count_header;
    obj.data = (uint8_t*) malloc(obj.size);

    uint8_t* write_ptr = obj.data;
    write_ptr[0] = byte_count_consts;
    write_ptr[1] = byte_count_consts >> 8;
    write_ptr[2] = byte_count_code;
    write_ptr[3] = byte_count_code >> 8;

    write_ptr += 4;

    // write the const section
    memcpy(write_ptr, const_store.values, byte_count_consts);
    write_ptr += byte_count_consts;

    // write the code
    memcpy(write_ptr, code_section.data, byte_count_code);
    
    u8buffer_destroy(&code_section);
    valbuffer_destroy(&const_store);
    parser_destroy(parser);
    
    return obj;

on_error:
    u8buffer_destroy(&code_section);
    valbuffer_destroy(&const_store);
    parser_destroy(parser);
    gvm_print_if_error(result_code, "asm_assemble");
    return (byte_code_block_t) { 0 };
}

int get_disasm_scheme_index(gvm_op_t op) {
    int nschemes = sizeof(schemes) / sizeof(schemes[0]);
    for(int i = 0; i < nschemes; i++) {
        if( op == schemes[i].opcode ) {
            return i;
        }
    }
    return -1;
}

void asm_debug_disassemble_code_object(byte_code_block_t* code_object) {
    int current_byte = 0;
    int current_instruction = 0;
    byte_code_header_t h = asm_read_byte_code_header(code_object);
    val_t* consts = (val_t*) (code_object->data + h.header_size);
    uint8_t* instructions = code_object->data + h.header_size + h.const_bytes;
    while( current_byte < h.code_bytes ) {
        gvm_op_t opcode = instructions[current_byte];
        int scheme_index = get_disasm_scheme_index(opcode);
        if( scheme_index < 0 ) {
            printf("<op %i not found>", opcode);
            current_byte ++;
            continue;
        }
        op_scheme_t scheme = schemes[scheme_index];
        char* name = scheme.name;
        printf("#%3i > %s", current_byte, name);
        current_byte ++;
        int arg_count = scheme_get_arg_count(scheme.typespec);
        for (int i = 0; i < arg_count; i++) {
            int val = READ_I16(instructions, current_byte);
            printf(" %i", val);
            current_byte += 2;
            if( scheme_is_flag_set(scheme.isconst, i) ) {
                printf(" (");
                val_print_lookup_val_array(consts, consts[val]);
                printf(")");
            }
        }
        printf("\n");
        current_instruction ++;
    }
}

void asm_destroy_code_object(byte_code_block_t* code_object) {
    if( code_object == NULL ) {
        return;
    }
    if( code_object->data != NULL ) {
        free(code_object->data);
        code_object->data = NULL;
        code_object->size = 0;
    }
}

byte_code_header_t asm_read_byte_code_header(byte_code_block_t* code_obj) {
    if( code_obj->size < 4 ) {
        return ( byte_code_header_t ) { 0 };
    }
    byte_code_header_t header;
    header.const_bytes = code_obj->data[0] | code_obj->data[1] << 8;
    header.code_bytes  = code_obj->data[2] | code_obj->data[3] << 8;
    header.header_size = 4;
    return header;
}