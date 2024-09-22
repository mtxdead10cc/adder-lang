#include "gvm_asm.h"
#include "gvm_parser.h"
#include "gvm_value.h"
#include "gvm.h"
#include "gvm_utils.h"
#include "gvm_memory.h"
#include "gvm_config.h"
#include "gvm_asmutils.h"
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
//  [string name]      [op bytecode id]    [arg type]                       [store as const]   [label reference]
    {"push",            OP_PUSH_VALUE,      ARGSPEC1(TT_STRING),             0x01,              0x00 },
    {"push",            OP_PUSH_VALUE,      ARGSPEC1(TT_NUMBER),             0x01,              0x00 },
    {"push",            OP_PUSH_VALUE,      ARGSPEC1(TT_VEC2),               0x01,              0x00 },
    {"store",           OP_STORE_LOCAL,     ARGSPEC1(TT_NUMBER),             0x00,              0x00 },
    {"load",            OP_LOAD_LOCAL,      ARGSPEC1(TT_NUMBER),             0x00,              0x00 },
    {"print",           OP_PRINT,           ARGSPEC1(0),                     0x00,              0x00 },
    {"call",            OP_CALL,            ARGSPEC1(TT_SYMBOL),             0x00,              0x01 },
    {"frame",           OP_MAKE_FRAME,      ARGSPEC2(TT_NUMBER, TT_NUMBER),  0x00,              0x00 },
    {"array",           OP_MAKE_ARRAY,      ARGSPEC1(0),                     0x00,              0x00 },
    {"array-len",       OP_ARRAY_LENGTH,    ARGSPEC1(0),                     0x00,              0x00 },
    {"iter",            OP_MAKE_ITER,       ARGSPEC1(0),                     0x00,              0x00 },
    {"iter-next",       OP_ITER_NEXT,       ARGSPEC1(TT_SYMBOL),             0x00,              0x01 },
    {"return",          OP_RETURN,          ARGSPEC1(0),                     0x00,              0x00 },
    {"pop1",            OP_POP_1,           ARGSPEC1(0),                     0x00,              0x00 },
    {"pop2",            OP_POP_2,           ARGSPEC1(0),                     0x00,              0x00 },
    {"dup1",            OP_DUP_1,           ARGSPEC1(0),                     0x00,              0x00 },
    {"dup2",            OP_DUP_2,           ARGSPEC1(0),                     0x00,              0x00 },
    {"rot2",            OP_ROT_2,           ARGSPEC1(0),                     0x00,              0x00 },
    {"is-less",         OP_CMP_LESS_THAN,   ARGSPEC1(0),                     0x00,              0x00 },
    {"is-more",         OP_CMP_MORE_THAN,   ARGSPEC1(0),                     0x00,              0x00 },
    {"is-equal",        OP_CMP_EQUAL,       ARGSPEC1(0),                     0x00,              0x00 },
    {"if-false",        OP_JUMP_IF_FALSE,   ARGSPEC1(TT_SYMBOL),             0x00,              0x01 },
    {"jump",            OP_JUMP,            ARGSPEC1(TT_SYMBOL),             0x00,              0x01 },
    {"exit",            OP_EXIT,            ARGSPEC1(TT_NUMBER),             0x00,              0x00 },
    {"and",             OP_AND,             ARGSPEC1(0),                     0x00,              0x00 },
    {"or",              OP_OR,              ARGSPEC1(0),                     0x00,              0x00 },
    {"mul",             OP_MUL,             ARGSPEC1(0),                     0x00,              0x00 },
    {"add",             OP_ADD,             ARGSPEC1(0),                     0x00,              0x00 },
    {"sub",             OP_SUB,             ARGSPEC1(0),                     0x00,              0x00 },
    {"neg",             OP_NEG,             ARGSPEC1(0),                     0x00,              0x00 },
    {"ncall",           OP_CALL_NATIVE,     ARGSPEC1(TT_SYMBOL),             0x01,              0x00 },
    {"init",            OP_ENTRY_POINT,            ARGSPEC1(TT_SYMBOL),             0x00,              0x01 }
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

    assert(OP_OPCODE_COUNT == 32 && "Opcode count changed (update schemes[]).");

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
            const_index = au_consts_add_string(consts, text);
        } break;
        case TT_NUMBER: {
            float value = parser_get_token_float_value(parser, token);
            const_index = au_consts_add_number(consts, value);
        } break;
        case TT_VEC2: {
            const_index = au_consts_add_ivec2(consts, text);
        } break;
        case TT_SYMBOL: {
            bool is_bool_true = false;
            bool is_bool_false = false;
            if( text_len <= 5 && text_len >= 4 ) {
                is_bool_true = strncmp(text, "true", 4) == 0;
                is_bool_false = strncmp(text, "false", 5) == 0;
            }
            if( is_bool_false ) {
                const_index = au_consts_add_bool(consts, false);
            } else if ( is_bool_true ) {
                const_index = au_consts_add_bool(consts, true);
            } else {
                const_index = au_consts_add_symbol_as_string(consts, text, text_len);
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

bool label_add(label_set_t* set, char* str, int len, int address) {

    if( set->count > GVM_ASM_MAX_LABELS ) {
        printf("error: hit max labels threashold\n");
        return false;
    }

    // check that the label name is not a reserved name 
    int nschemes = sizeof(schemes) / sizeof(schemes[0]);
    for(int i = 0; i < nschemes; i++) {
        if(strncmp(schemes[i].name, str, len) == 0) {
            printf("error: invalid label name, '%.*s' is a reserved keyword.\n", len, str);
            return false;
        }
    }

    for(int i = 0; i < set->count; i++) {
        char* existing = set->label[i];
        int len = set->length[i];
        // already added
        if( strncmp(existing, str, len) == 0 ) {
            printf("error: duplicate label definition '%.*s'.\n", len, existing);
            return false;
        }
    }

    set->label[set->count] = str;
    set->length[set->count] = len;
    set->address[set->count] = address;
    set->count ++;
    return true;
}

int label_get_address(label_set_t* set, char* str, int len) {
    for(int i = 0; i < set->count; i++) {
        if( strncmp(set->label[i], str, len) == 0 ) {
            return set->address[i];
        }
    }
    return -1;
}

/* [asm_scan_labels]
    Runs through all the tokens counting bytes in order to 
    keep track of op addresses. For every label encountered
    the label name and address (byte index) is stored in
    the provided label_set. */
bool asm_scan_labels(parser_t* parser, label_set_t* label_set) {

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
            bool res = label_add(label_set, label, length, address);
            if( res == false ) {
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
            return false;
        }

    }

    return true;
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
gvm_program_t asm_assemble_code_object(char* code_buffer) {
    
    parser_t* parser = parser_create(code_buffer);
    if( parser == NULL ) {
        return (gvm_program_t) { 0 };
    }

    label_set_t label_set = { 0 };
    u8buffer_t code_section = { 0 };
    valbuffer_t const_store = { 0 };

#if GVM_TRACE_LOG_LEVEL >= 4
    printf("START TOKENS\n");
    parser_debug_print_tokens(parser);
    printf("END TOKENS\n");
#endif

    bool res = asm_scan_labels(parser, &label_set);
    if( res == false ) {
        goto on_error;
    }

    parser_reset(parser);

    if( valbuffer_create(&const_store, 5) == false ) {
        goto on_error;
    }
    
    if( u8buffer_create(&code_section, 16) == false ) {
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
                        goto on_error;
                    }
                    DBG_LOG("%i (%.*s) ", label_index, len, ptr);
                    assert(label_index >= 0 && label_index < 256);
                    u8buffer_write_i16(&code_section, label_index);
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

    gvm_program_t prog = { 0 };
    prog.cons.count = const_store.size;
    prog.cons.buffer = (val_t*) malloc(sizeof(val_t) * const_store.size);
    memcpy(prog.cons.buffer, const_store.values, sizeof(val_t) * const_store.size);

    prog.inst.size = code_section.size;
    prog.inst.buffer = (uint8_t*) malloc(code_section.size * sizeof(uint8_t));
    memcpy(prog.inst.buffer, code_section.data, code_section.size * sizeof(uint8_t));

    u8buffer_destroy(&code_section);
    valbuffer_destroy(&const_store);
    parser_destroy(parser);
    
    return prog;

on_error:
    u8buffer_destroy(&code_section);
    valbuffer_destroy(&const_store);
    parser_destroy(parser);
    return (gvm_program_t) { 0 };
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

void asm_debug_disassemble_code_object(gvm_program_t* code_object) {
    int current_byte = 0;
    int current_instruction = 0;
    val_t* consts = code_object->cons.buffer;
    uint8_t* instructions = code_object->inst.buffer;
    int instr_byte_count = code_object->inst.size;
    while( current_byte < instr_byte_count ) {
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

void asm_destroy_code_object(gvm_program_t* code_object) {
    if( code_object == NULL ) {
        return;
    }
    if( code_object->cons.buffer != NULL ) {
        free(code_object->cons.buffer);
        code_object->cons.count = 0;
        code_object->cons.buffer = NULL;
    }
    if( code_object->inst.buffer != NULL ) {
        free(code_object->inst.buffer);
        code_object->inst.size = 0;
        code_object->inst.buffer = NULL;
    }
}