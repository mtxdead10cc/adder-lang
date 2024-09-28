#ifndef GVM_TOKENIZER_H_
#define GVM_TOKENIZER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "gvm_utils.h"
#include "gvm_lexer.h"

typedef enum token_type_t {
    TT_INITIAL,
    TT_SPACE,
    TT_COMMENT,
    TT_SYMBOL,
    TT_NUMBER,
    TT_BOOLEAN,
    TT_SEPARATOR,
    TT_STATEMENT_END,
    TT_STRING,
    TT_GROUP_START,
    TT_GROUP_END,
    TT_ARROW,
    TT_ASSIGN,
    TT_KW_IF,
    TT_KW_ELSE,
    TT_KW_FOR,
    TT_KW_RETURN,
    TT_KW_FUN_DEF,
    TT_CMP_EQ,
    TT_CMP_GT,
    TT_CMP_LT,
    TT_CMP_GT_EQ,
    TT_CMP_LT_EQ,
    TT_FINAL,
    TT_TOKEN_TYPE_COUNT
} token_type_t;

typedef struct token_t {
    token_type_t type;
    srcref_t ref;
} token_t;

typedef struct token_collection_t {
    token_t*    tokens;
    size_t      capacity;
    size_t      count;
} token_collection_t;

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
        case TT_GROUP_START: return "TT_GROUP_START";
        case TT_GROUP_END: return "TT_GROUP_END";
        case TT_ARROW: return "TT_ARROW";
        case TT_ASSIGN: return "TT_ASSIGN";
        case TT_KW_IF: return "TT_KW_IF";
        case TT_KW_ELSE: return "TT_KW_ELSE";
        case TT_KW_FOR: return "TT_KW_FOR";
        case TT_KW_RETURN: return "TT_KW_RETURN";
        case TT_KW_FUN_DEF: return "TT_KW_FUN_DEF";
        case TT_CMP_EQ: return "TT_CMP_EQ";
        case TT_CMP_GT: return "TT_CMP_GT";
        case TT_CMP_LT: return "TT_CMP_LT";
        case TT_CMP_GT_EQ: return "TT_CMP_GT_EQ";
        case TT_CMP_LT_EQ: return "TT_CMP_LT_EQ";
        case TT_FINAL: return "TT_FINAL";
        default: return "<UNKNOWN-TT>";
    }
}

bool tokens_init(token_collection_t* collection, size_t capacity);
void tokens_clear(token_collection_t* collection);
bool tokens_append(token_collection_t* collection, token_t token);
void tokens_print(token_collection_t* collection);
void tokens_destroy(token_collection_t* collection);
bool tokenizer_analyze(token_collection_t* collection, char* text, size_t text_length, char* filepath);

#endif // GVM_TOKENIZER_H_