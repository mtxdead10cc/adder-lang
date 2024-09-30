#ifndef GVM_TOKENIZER_H_
#define GVM_TOKENIZER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "gvm_utils.h"
#include "gvm_lexer.h"



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
        case TT_INITIAL: return "TT_INITIAL";
        case TT_SPACE: return "TT_SPACE";
        case TT_COMMENT: return "TT_COMMENT";
        case TT_SYMBOL: return "TT_SYMBOL";
        case TT_NUMBER: return "TT_NUMBER";
        case TT_BOOLEAN: return "TT_BOOLEAN";
        case TT_SEPARATOR: return "TT_SEPARATOR";
        case TT_STATEMENT_END: return "TT_STATEMENT_END";
        case TT_STRING: return "TT_STRING";
        case TT_ARROW: return "TT_ARROW";
        case TT_ASSIGN: return "TT_ASSIGN";
        case TT_KW_IF: return "TT_KW_IF";
        case TT_KW_ELSE: return "TT_KW_ELSE";
        case TT_KW_FOR: return "TT_KW_FOR";
        case TT_KW_RETURN: return "TT_KW_RETURN";
        case TT_KW_FUN_DEF: return "TT_KW_FUN_DEF";
        case TT_KW_AND: return "TT_KW_AND";
        case TT_KW_OR: return "TT_KW_OR";
        case TT_KW_NOT: return "TT_KW_NOT";
        case TT_CMP_EQ: return "TT_CMP_EQ";
        case TT_CMP_GT_EQ: return "TT_CMP_GT_EQ";
        case TT_CMP_LT_EQ: return "TT_CMP_LT_EQ";
        case TT_LT_OR_OPEN_ABRACKET: return "TT_LT_OR_OPEN_ABRACKET";
        case TT_GT_OR_CLOSE_ABRACKET: return "TT_GT_OR_CLOSE_ABRACKET";
        case TT_OPEN_PAREN: return "TT_OPEN_PAREN";
        case TT_CLOSE_PAREN: return "TT_CLOSE_PAREN";
        case TT_OPEN_CURLY: return "TT_OPEN_CURLY";
        case TT_CLOSE_CURLY: return "TT_CLOSE_CURLY";
        case TT_OPEN_SBRACKET: return "TT_OPEN_SBRACKET";
        case TT_CLOSE_SBRACKET: return "TT_CLOSE_SBRACKET";
        case TT_FINAL: return "TT_FINAL";
        default: return "<UNKNOWN-TT>";
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
} tokenizer_args_t;

bool tokenizer_analyze(token_collection_t* collection, tokenizer_args_t* args);

#endif // GVM_TOKENIZER_H_