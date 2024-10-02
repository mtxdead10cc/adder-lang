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

build_result_t pa_init(parser_t* parser, char* text, size_t text_length, char* filepath) {

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
    if( r_is_error(parser->result) ) {
        tokens_destroy(&parser->collection);
        parser->collection.count = 0;
        return parser->result;
    }

    parser->cursor = 0;
    parser->result.code = R_OK;
    return parser->result;
}

void pa_destroy(parser_t* parser) {
    if( parser == NULL ) {
        return;
    }
    tokens_destroy(&parser->collection);
}

bool pa_is_at_end(parser_t* parser) {
    return parser->cursor >= (parser->collection.count - 1);
}

bool pa_advance(parser_t* parser) {
    if( pa_is_at_end(parser) == false ) {
        parser->cursor ++;
        return true;
    }
    return false;
}

bool pa_advance_if(parser_t* parser, token_type_t mask) {
    if( (pa_current_token(parser).type & mask) > 0 ) {
        return pa_advance(parser);
    }
    return false;
}

bool pa_advance_if_not(parser_t* parser, token_type_t mask) {
    if( (pa_current_token(parser).type & mask) == 0 ) {
        return pa_advance(parser);
    }
    return false;
}

token_t pa_current_token(parser_t* parser) {
    return parser->collection.tokens[parser->cursor];
}

token_t pa_peek_token(parser_t* parser, int lookahead) {
    int diff = parser->collection.count - parser->cursor;
    if( diff < lookahead  ) {
        lookahead = diff;
    }
    return parser->collection.tokens[parser->cursor + lookahead];
}

srcref_location_t pa_get_location(parser_t* parser, int cursor_offset) {
    srcref_t ref = parser->collection.tokens[parser->cursor + cursor_offset].ref;
    char* filepath = parser->filepath;
    return srcref_location(ref, filepath);
}

pa_result_t pa_set_build_error(parser_t* parser, build_result_t result) {
    if( r_is_error(parser->result) ) { // return pre-existing
        return par_error(&parser->result);
    }
    parser->result = result;
    return par_error(&parser->result);
}

pa_result_t pa_consume(parser_t* parser, token_type_t expected) {
    if( pa_is_at_end(parser) ) {
        return par_out_of_tokens();
    }
    token_type_t actual = pa_current_token(parser).type;
    if( ((uint32_t) expected & (uint32_t) actual) == 0 ) {
        return pa_set_build_error(parser,
                                    r_unexpected_token(
                                        pa_get_location(parser, 0),
                                        expected, actual));
    }
    if( pa_advance(parser) == false ) {
        return par_out_of_tokens();
    }
    return par_nothing();
}

pa_result_t pa_parse_expression(parser_t* parser);

pa_result_t pa_parse_number(parser_t* parser) {
    token_t token = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_NUMBER);
    if( par_is_nothing(result) == false ) {
        return result;
    }
    float value = 0.0f;
    if( srcref_as_float(token.ref, &value) ) {
        return par_node(ast_number(value));
    }
    return pa_set_build_error(parser,
                    r_invalid_format(
                        srcref_location(token.ref,
                            parser->filepath)));
}

pa_result_t pa_parse_boolean(parser_t* parser) {
    token_t token = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_BOOLEAN);
    if( par_is_nothing(result) == false ) {
        return result;
    }
    bool value = false;
    if( srcref_as_bool(token.ref, &value) ) {
        return par_node(ast_bool(value));
    }
    return pa_set_build_error(parser,
                    r_invalid_format(
                        srcref_location(token.ref,
                            parser->filepath)));
}

pa_result_t pa_parse_string(parser_t* parser) {
    token_t token = pa_current_token(parser);
    pa_result_t result = pa_consume(parser, TT_STRING);
    if( par_is_nothing(result) == false ) {
        return result;
    }
    return par_node(ast_string(token.ref));
}

pa_result_t pa_try_parse_value(parser_t* parser) {
    token_t token = pa_current_token(parser);
    if( token.type == TT_BOOLEAN ) {
        return pa_parse_boolean(parser);
    } else if ( token.type == TT_NUMBER ) {
        return pa_parse_number(parser);
    } else if ( token.type == TT_STRING ) {
        return pa_parse_string(parser);
    }
    return par_nothing();
}

pa_result_t pa_try_parse_var_name(parser_t* parser) {
    token_t token = pa_current_token(parser);
    if( token.type == TT_SYMBOL ) {
        return par_node(ast_varref(token.ref));
    }
    return par_nothing();
}

pa_result_t pa_try_parse_func_call(parser_t* parser) {

    token_t func_name = pa_current_token(parser);
    token_t open_paren = pa_peek_token(parser, 1);
    
    if( func_name.type != TT_SYMBOL || open_paren.type != TT_OPEN_PAREN ) {
        return par_nothing();
    }

    if( pa_advance(parser) == false ) {
        return par_out_of_tokens();
    }

    if( pa_advance(parser) == false ) {
        return par_out_of_tokens();
    }
    
    ast_node_t* args = ast_block();
    do {
        pa_result_t expr_res = pa_parse_expression(parser);
        if( par_is_node(expr_res) == false ) {
            ast_free(args);
            return expr_res;
        }
        ast_block_add(args, par_extract_node(expr_res));
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_consume(parser, TT_CLOSE_PAREN);

    return par_node(ast_funcall(func_name.ref, args));
}

// TODO: need to support grouping (a + b) * z inside the ast and the VM

pa_result_t pa_parse_expression(parser_t* parser) {

    pa_result_t result = pa_try_parse_func_call(parser);

    if( par_is_node(result) ) {
        return result;
    }

    result = pa_try_parse_value(parser);
    if( par_is_node(result) ) {
        return result;
    }

    result = pa_try_parse_var_name(parser);
    if( par_is_node(result) ) {
        return result;
    }
    // TODO: INTRODUCE INVALID EXPRESSION ERROR
    return pa_set_build_error(parser,
                    r_unexpected_token(
                        pa_get_location(parser, 0),
                        TT_SYMBOL|TT_BOOLEAN|TT_STRING, // etc.
                        pa_current_token(parser).type));
}
