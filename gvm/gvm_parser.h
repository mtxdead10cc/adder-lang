#ifndef GVM_PARSER_H_
#define GVM_PARSER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include "gvm_types.h"
#include "gvm_utils.h"
#include "gvm_lexer.h"

typedef enum token_type_t {
    TT_INITIAL,
    TT_SPACE,
    TT_COMMENT,
    TT_SYMBOL,
    TT_NUMBER,
    TT_BOOLEAN,
    TT_STRING,
    TT_GROUP_START,
    TT_GROUP_END,
    TT_FINAL,
    TT_TOKEN_TYPE_COUNT
} token_type_t;

typedef struct token_t {
    token_type_t type;
    srcref_t ref;
} token_t;

typedef struct parser_t {
    token_t*    tokens;
    size_t      token_capacity;
    size_t      token_count;
    size_t      token_index;
} parser_t;

bool parser_init(parser_t* parser, char* text, size_t text_length, char* filepath);
void parser_dump_tokens(parser_t* parser);

#endif // GVM_PARSER_H_