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

build_result_t parser_init(parser_t* parser, char* text, size_t text_length, char* filepath) {

    parser->result = (build_result_t) {0};
    parser->filepath = filepath;
    
    if( tokens_init(&parser->collection, 16) == false ) {
        parser->result.code = R_ER_OUT_OF_MEMORY;
        return parser->result;
    }

    if( text == NULL ) {
        parser->result.code = R_ER_INVALID_STATE;
        return parser->result;
    }

    tokenizer_args_t args = (tokenizer_args_t) {
        .filepath = filepath,
        .text = text,
        .text_length = text_length,
        .include_comments = false,
        .include_spaces = false
    };

    parser->result = tokenizer_analyze(&parser->collection, &args);
    if( res_is_error(parser->result) ) {
        tokens_destroy(&parser->collection);
        parser->collection.count = 0;
        return parser->result;
    }

    parser->cursor = 0;
    parser->result.code = R_OK;
    return parser->result;
}

void parser_destroy(parser_t* parser) {
    if( parser == NULL ) {
        return;
    }
    tokens_destroy(&parser->collection);
}

bool parser_is_at_end(parser_t* parser) {
    if( res_is_error(parser->result) ) {
        return true;
    }
    return parser->cursor >= (parser->collection.count - 1);
}

bool parser_advance(parser_t* parser) {
    if( parser_is_at_end(parser) == false ) {
        parser->cursor ++;
        return true;
    }
    return false;
}

token_type_t parser_current_token_type(parser_t* parser) {
    return parser->collection.tokens[parser->cursor].type;
}

srcref_t parser_current_srcref(parser_t* parser) {
    return parser->collection.tokens[parser->cursor].ref;
}

srcref_location_t parser_current_location(parser_t* parser) {
    srcref_t ref = parser->collection.tokens[parser->cursor].ref;
    char* filepath = parser->filepath;
    return srcref_location_of(ref, filepath);
}

bool parser_consume(parser_t* parser, token_type_t expected) {
    if( parser_is_at_end(parser) ) {
        return false;
    }
    token_type_t actual = parser_current_token_type(parser);
    if( ((uint32_t) expected & (uint32_t) actual) == 0 ) {
        parser->result = res_err_unexpected_token(
            parser_current_location(parser),
            expected, actual);
        return false;
    }
    return parser_advance(parser);
}
