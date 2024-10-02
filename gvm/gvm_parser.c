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
#include "gvm_ast.h"

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

bool parser_advance_if(parser_t* parser, token_type_t mask) {
    if( (parser_current_token(parser).type & mask) > 0 ) {
        return parser_advance(parser);
    }
    return false;
}

bool parser_advance_if_not(parser_t* parser, token_type_t mask) {
    if( (parser_current_token(parser).type & mask) == 0 ) {
        return parser_advance(parser);
    }
    return false;
}

token_t parser_current_token(parser_t* parser) {
    return parser->collection.tokens[parser->cursor];
}

token_t parser_peek_token(parser_t* parser, int lookahead) {
    int diff = parser->collection.count - parser->cursor;
    if( diff < lookahead  ) {
        lookahead = diff;
    }
    return parser->collection.tokens[parser->cursor + lookahead];
}

srcref_location_t parser_current_location(parser_t* parser) {
    srcref_t ref = parser->collection.tokens[parser->cursor].ref;
    char* filepath = parser->filepath;
    return srcref_location(ref, filepath);
}

bool parser_consume(parser_t* parser, token_type_t expected) {
    if( parser_is_at_end(parser) ) {
        return false;
    }
    token_type_t actual = parser_current_token(parser).type;
    if( ((uint32_t) expected & (uint32_t) actual) == 0 ) {
        parser->result = res_err_unexpected_token(
            parser_current_location(parser),
            expected, actual);
        return false;
    }
    return parser_advance(parser);
}

ast_node_t* parse_expression(parser_t* parser);

ast_node_t* parse_number(parser_t* parser) {
    token_t token = parser_current_token(parser);
    if( parser_consume(parser, TT_NUMBER) == false ) {
        return NULL;
    }
    float value = 0.0f;
    if( srcref_as_float(token.ref, &value) ) {
        return ast_number(value);
    }
    parser->result = res_err_invalid_token_format(
        srcref_location(token.ref, parser->filepath));
    return NULL;
}

ast_node_t* parse_boolean(parser_t* parser) {
    token_t token = parser_current_token(parser);
    if( parser_consume(parser, TT_BOOLEAN) == false ) {
        return NULL;
    }
    bool value = false;
    if( srcref_as_bool(token.ref, &value) ) {
        return ast_bool(value);
    }
    parser->result = res_err_invalid_token_format(
        srcref_location(token.ref, parser->filepath));
    return NULL;
}

ast_node_t* parse_string(parser_t* parser) {
    token_t token = parser_current_token(parser);
    if( parser_consume(parser, TT_STRING) ) {
        return ast_string(token.ref);
    }
    return NULL;
}

ast_node_t* try_parse_value(parser_t* parser) {
    token_t token = parser_current_token(parser);
    if( token.type == TT_BOOLEAN ) {
        return parse_boolean(parser);
    } else if ( token.type == TT_NUMBER ) {
        return parse_number(parser);
    } else if ( token.type == TT_STRING ) {
        return parse_string(parser);
    }
    return NULL;
}

ast_node_t* try_parse_var_name(parser_t* parser) {
    token_t token = parser_current_token(parser);
    if( token.type == TT_SYMBOL ) {
        return ast_varref(token.ref);
    }
    return NULL;
}

ast_node_t* try_parse_func_call(parser_t* parser) {

    token_t func_name = parser_current_token(parser);
    token_t open_paren = parser_peek_token(parser, 1);
    
    if( func_name.type != TT_SYMBOL || open_paren.type != TT_OPEN_PAREN ) {
        return NULL;
    }

    parser_advance(parser);
    parser_advance(parser);
    
    ast_node_t* args = ast_block();
    do {
        ast_block_add(args, parse_expression(parser));
    } while( parser_advance_if(parser, TT_SEPARATOR) );

    parser_consume(parser, TT_CLOSE_PAREN);

    return ast_funcall(func_name.ref, args);
}

// TODO: need to support grouping (a + b) * z inside the ast and the VM

ast_node_t* parse_expression(parser_t* parser) {
    ast_node_t* result = try_parse_func_call(parser);
    if( result != NULL ) {
        return result;
    }
    result = try_parse_value(parser);
    if( result != NULL ) {
        return result;
    }
    result = try_parse_var_name(parser);
    if( result != NULL ) {
        return result;
    }
    // todo: if null maybe its an error?
    return NULL;
}
