#ifndef GVM_TOKENIZER_H_
#define GVM_TOKENIZER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "sh_utils.h"
#include "co_lexer.h"
#include "co_types.h"
#include "co_utils.h"

inline static token_t token_const(const char* content, token_type_t type) {
    return (token_t) {
        .ref = srcref_const(content),
        .type = type
    };
}

inline static bool token_equals(token_t a, token_t b) {
    return a.type == b.type && srcref_equals(a.ref, b.ref);
}

inline static char* token_get_type_name(token_type_t type) {
    switch (type) {
        case TT_NOTHING: return "TT_NOTHING";
        case TT_INITIAL: return "TT_INITIAL";
        case TT_SPACE: return "TT_SPACE";
        case TT_COMMENT: return "TT_COMMENT";
        case TT_SYMBOL: return "TT_SYMBOL";
        case TT_NUMBER: return "TT_NUMBER";
        case TT_BOOLEAN: return "TT_BOOLEAN";
        case TT_STRING: return "TT_STRING";
        case TT_ARROW: return "TT_ARROW";
        case TT_ASSIGN: return "TT_ASSIGN";
        case TT_KW_IF: return "TT_KW_IF";
        case TT_KW_ELSE: return "TT_KW_ELSE";
        case TT_KW_FOR: return "TT_KW_FOR";
        case TT_KW_IN: return "TT_KW_IN";
        case TT_KW_RETURN: return "TT_KW_RETURN";
        case TT_KW_BREAK: return "TT_KW_BREAK";
        case TT_KW_FUN_DEF: return "TT_KW_FUN_DEF";
        case TT_CMP_EQ: return "TT_CMP_EQ";
        case TT_CMP_NEQ: return "TT_CMP_NEQ";
        case TT_CMP_GT_EQ: return "TT_CMP_GT_EQ";
        case TT_CMP_LT_EQ: return "TT_CMP_LT_EQ";
        case TT_CMP_GT: return "TT_CMP_GT";
        case TT_CMP_LT: return "TT_CMP_LT";
        case TT_OPEN_PAREN: return "TT_OPEN_PAREN";
        case TT_CLOSE_PAREN: return "TT_CLOSE_PAREN";
        case TT_OPEN_CURLY: return "TT_OPEN_CURLY";
        case TT_CLOSE_CURLY: return "TT_CLOSE_CURLY";
        case TT_OPEN_SBRACKET: return "TT_OPEN_SBRACKET";
        case TT_CLOSE_SBRACKET: return "TT_CLOSE_SBRACKET";
        case TT_UNOP_NOT: return "TT_UNOP_NOT";
        case TT_BINOP_AND: return "TT_BINOP_AND";
        case TT_BINOP_OR: return "TT_BINOP_OR";
        case TT_BINOP_MUL: return "TT_BINOP_MUL";
        case TT_BINOP_DIV: return "TT_BINOP_DIV";
        case TT_BINOP_MOD: return "TT_BINOP_MOD";
        case TT_BINOP_PLUS: return "TT_BINOP_PLUS";
        case TT_BINOP_MINUS: return "TT_BINOP_MINUS";
        case TT_SEPARATOR: return "TT_SEPARATOR";
        case TT_HASH_SIGN: return "TT_HASH_SIGN";
        case TT_STATEMENT_END: return "TT_STATEMENT_END";
        case TT_IMPORT: return "TT_IMPORT";
        case TT_EXPORT: return "TT_EXPORT";
        case TT_FINAL: return "TT_FINAL";
        default: return "<UNKNOWN-TT>";
    }
}

inline static bool token_is_binary_operation(token_type_t type) {
    switch (type) {
        case TT_BINOP_AND:
        case TT_BINOP_OR:
        case TT_BINOP_MUL:
        case TT_BINOP_DIV:
        case TT_BINOP_MOD:
        case TT_BINOP_PLUS:
        case TT_BINOP_MINUS: return true;
        default: return false;
    }
}

bool tokens_init(token_collection_t* collection, size_t capacity);
void tokens_clear(token_collection_t* collection);
bool tokens_append(token_collection_t* collection, token_t token);
void tokens_print(token_collection_t* collection);
void tokens_destroy(token_collection_t* collection);

typedef struct tokenizer_args_t {
    bool include_comments;
    bool include_spaces;
    char* text;
    size_t text_length;
    char* filepath;
    trace_t* trace;
} tokenizer_args_t;

bool tokenizer_analyze(token_collection_t* collection, tokenizer_args_t* args);

#endif // GVM_TOKENIZER_H_
