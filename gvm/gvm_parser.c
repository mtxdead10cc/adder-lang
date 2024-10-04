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
#include "gvm_cres.h"
#include "gvm_ast.h"

bool pa_init(parser_t* parser, char* text, size_t text_length, char* filepath) {

    parser->result = (cres_t) {0};

    cres_set_src_filepath(&parser->result, filepath);
    
    if( tokens_init(&parser->collection, 16) == false ) {
        cres_set_error(&parser->result, R_ERR_OUT_OF_MEMORY);
        return false;
    }

    if( text == NULL ) {
        cres_set_error(&parser->result, R_ERR_INTERNAL);
        return false;
    }

    tokenizer_args_t args = (tokenizer_args_t) {
        .filepath = filepath,
        .text = text,
        .text_length = text_length,
        .include_comments = false,
        .include_spaces = false,
        .resultptr = &parser->result
    };

    if( tokenizer_analyze(&parser->collection, &args) == false ) {
        tokens_destroy(&parser->collection);
        parser->collection.count = 0;
        return false;
    }

    parser->cursor = 0;
    return cres_is_ok(&parser->result);
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


void pa_set_error_unexpected_token_type(parser_t* parser, token_type_t expected, token_t actual) {
    if( cres_set_error(&parser->result, R_ERR_TOKEN) ) {
        cres_set_src_location(&parser->result, actual.ref);
        cres_msg_add_costr(&parser->result, "unexpected token, expected ");
        cres_msg_add_token_type_name(&parser->result, expected);
        cres_msg_add_costr(&parser->result, " but found ");
        cres_msg_add_token(&parser->result, actual);
    }
}

void pa_set_error_invalid_token_format(parser_t* parser, token_t token) {
    if( cres_set_error(&parser->result, R_ERR_TOKEN) ) {
        cres_set_src_location(&parser->result, token.ref);
        cres_msg_add_costr(&parser->result, "unexpected token format: ");
        cres_msg_add_token(&parser->result, token);
    }
}

pa_result_t pa_consume(parser_t* parser, token_type_t expected) {
    if( cres_has_error(&parser->result) ) {
        return par_error(&parser->result);
    }
    if( pa_is_at_end(parser) ) {
        return par_out_of_tokens();
    }
    token_type_t actual = pa_current_token(parser).type;
    if( ((uint32_t) expected & (uint32_t) actual) == 0 ) {
        return par_error(&parser->result);
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
    pa_set_error_invalid_token_format(parser, token);
    return par_error(&parser->result);
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
    pa_set_error_invalid_token_format(parser, token);
    return par_error(&parser->result);
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

    pa_result_t result = pa_consume(parser, TT_CLOSE_PAREN);
    if( par_is_nothing(result) == false ) {
        ast_free(args);
        return result;
    } else {
        return par_node(ast_funcall(func_name.ref, args));
    }
}

pa_result_t pa_try_parse_array_def(parser_t* parser) {

    if( pa_current_token(parser).type != TT_OPEN_SBRACKET ) {
        return par_nothing();
    }

    if( pa_advance(parser) == false ) {
        return par_out_of_tokens();
    }

    ast_node_t* array = ast_array();

    do {
        pa_result_t expr_res = pa_parse_expression(parser);
        if( par_is_node(expr_res) == false ) {
            ast_free(array);
            return expr_res;
        }
        ast_block_add(array, par_extract_node(expr_res));
    } while( pa_advance_if(parser, TT_SEPARATOR) );

    pa_result_t result = pa_consume(parser, TT_CLOSE_SBRACKET);

    if( par_is_nothing(result) == false ) {
        ast_free(array);
        return result;
    } else {
        return par_node(array);
    }
}

// https://www.reddit.com/r/ProgrammingLanguages/comments/1c9cjjg/best_way_to_parse_binary_operations/


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

    result = pa_try_parse_array_def(parser);
    if( par_is_node(result) ) {
        return result;
    }

    if( cres_set_error(&parser->result, R_ERR_EXPR) ) {
        token_t token = pa_current_token(parser);
        cres_set_src_location(&parser->result, token.ref);
        cres_msg_add_token(&parser->result, token);
        cres_msg_add_costr(&parser->result, " could not be made into an expression.");
    }
    return par_error(&parser->result);
}
