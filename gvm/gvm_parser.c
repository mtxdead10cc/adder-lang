#include "gvm_parser.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gvm_types.h"
#include "gvm_utils.h"
#include "gvm_tokenizer.h"
#include "gvm_build_result.h"


bool parser_init(parser_t* parser, char* text, size_t text_length, char* filepath) {

    parser->result = (build_result_t) {0};
    
    if( tokens_init(&parser->collection, 16) == false ) {
        parser->result.code = R_ER_HOST_OUT_OF_MEMORY;
        return false;
    }

    if( text == NULL ) {
        parser->result.code = R_ER_INVALID_STATE;
        return false;
    }

    tokenizer_args_t args = (tokenizer_args_t) {
        .filepath = filepath,
        .text = text,
        .text_length = text_length,
        .include_comments = false,
        .include_spaces = false
    };

    if( tokenizer_analyze(&parser->collection, &args) == false ) {
        tokens_destroy(&parser->collection);
        return false;
    }

    parser->cursor = 0;
    parser->result.code = R_OK;

    return true;
}

void parser_destroy(parser_t* parser) {
    if( parser == NULL ) {
        return;
    }
    tokens_destroy(&parser->collection);
}

bool parser_is_at_end(parser_t* parser) {
    return parser->cursor >= (parser->collection.count - 1);
}

void parser_advance(parser_t* parser) {
    if( parser_is_at_end(parser) == false ) {
        parser->cursor ++;
    }
}

token_type_t parser_current_token_type(parser_t* parser) {
    return parser->collection.tokens[parser->cursor].type;
}

srcref_t parser_current_srcref(parser_t* parser) {
    return parser->collection.tokens[parser->cursor].ref;
}

bool parser_consume(parser_t* parser, token_type_t expected) {
    token_type_t actual = parser_current_token_type(parser);
    if( ((uint32_t) expected & (uint32_t) actual) == 0 ) {
        parser->result = res_err_unexpected_token(
            parser_current_srcref(parser),
            expected, actual);
        return false;
    }
    parser_advance(parser);
    return true;
}
