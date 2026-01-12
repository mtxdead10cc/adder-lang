#include "adrcom/parser/co_parser.h"
#include "adrcom/parser/co_tokenizer.h"

#include <adrcom/shared/co_trace.h>

#include <adrcom/ast/co_ast.h>

#include <shared/sh_types.h>
#include <shared/sh_utils.h>
#include <shared/sh_log.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pa_result_t pa_init(parser_t* parser, arena_t* arena, trace_t* trace, char* text, size_t text_length, char* filepath) {

    parser->trace = trace;

    trace_set_current_source_path(trace, filepath);
    
    if( tokens_init(&parser->collection, 16) == false ) {
        trace_out_of_memory_error(trace);
        return par_error();
    }

    if( text == NULL ) {
        trace_msg_t* msg = trace_create_message(trace, TM_ERROR, trace_no_ref());
        trace_msg_append_costr(msg, "the input text buffer pointer was null.");
        return par_error();
    }

    tokenizer_args_t args = (tokenizer_args_t) {
        .filepath = filepath,
        .text = text,
        .text_length = text_length,
        .include_comments = false,
        .include_spaces = false,
        .trace = trace
    };

    if( tokenizer_analyze(&parser->collection, &args) == false ) {
        tokens_destroy(&parser->collection);
        parser->collection.count = 0;
        trace_msg_t* msg = trace_create_message(trace, TM_ERROR, trace_no_ref());
        trace_msg_append_costr(msg, "the input text buffer pointer was null.");
        return par_error();
    }

    parser->arena = arena;
    parser->cursor = 0;
    return par_nothing();
}

void pa_destroy(parser_t* parser) {
    if( parser == NULL ) {
        return;
    }
    tokens_destroy(&parser->collection);
}

bool pa_is_at_end(parser_t* parser) {
    return parser->cursor >= parser->collection.count;
}

bool pa_advance(parser_t* parser) {
    if( pa_is_at_end(parser) == false ) {

        parser->cursor ++;
        return true;
    }
    return false;
}

bool pa_advance_if(parser_t* parser, token_type_t type) {
    if( pa_current_token(parser).type == type ) {
        return pa_advance(parser);
    }
    return false;
}

bool pa_advance_if_not(parser_t* parser, token_type_t type) {
    if( pa_current_token(parser).type != type) {
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

pa_result_t pa_consume(parser_t* parser, token_type_t expected) {
    if( trace_get_error_count(parser->trace) > 0 ) {
        return par_error();
    }
    if( pa_is_at_end(parser) ) {
        return par_error_out_of_tokens(parser);
    }
    token_t actual = pa_current_token(parser);
    if( expected != actual.type ) {
        return par_error_unexpected_token_type(parser, expected, actual);
    }
    pa_advance(parser); // do not check eof here
    return par_nothing();
}

pa_result_t par_node(ast_t* node) {
    return (pa_result_t) {
        .type = PAR_AST_NODE,
        .node = node,
        .group_expression = false
    };
}

pa_result_t par_nothing(void) {
    return (pa_result_t) {
        .type = PAR_NOTHING,
        .node = NULL,
        .group_expression = false
    };
}

pa_result_t par_error(void) {
    return (pa_result_t) {
        .type = PAR_BUILD_ERROR,
        .node = NULL,
        .group_expression = false
    };
}

bool par_is_nothing(pa_result_t res) {
    return res.type == PAR_NOTHING;
}

bool par_is_error(pa_result_t res) {
    return res.type == PAR_BUILD_ERROR;
}

bool par_is_node(pa_result_t res) {
    return res.type == PAR_AST_NODE;
}

ast_t* par_extract_node(pa_result_t res) {
    assert(res.type == PAR_AST_NODE);
    return (ast_t*) res.node;
}

pa_result_t par_error_out_of_tokens(parser_t* parser) {
    trace_msg_t* msg = trace_create_message(parser->trace, TM_ERROR, trace_no_ref());
    trace_msg_append_costr(msg, "unexpected end of token stream.");
    return par_error();
}

pa_result_t par_error_unexpected_token_type(parser_t* parser, token_type_t expected, token_t actual) {
    trace_msg_t* msg = trace_create_message(parser->trace, TM_ERROR, actual.ref);
    trace_msg_append_costr(msg, "unexpected token, expected ");
    tokenizer_trace_msg_append_token_type_name(msg, expected);
    trace_msg_append_costr(msg, " but found ");
    tokenizer_trace_msg_append_token_type_name(msg, actual.type);
    trace_msg_append_costr(msg, " ('");
    trace_msg_append(msg,
        srcref_ptr(actual.ref),
        srcref_len(actual.ref));
    trace_msg_append_costr(msg, "')");
    return par_error();
}

pa_result_t par_error_invalid_token_format(parser_t* parser, token_t token) {
    trace_msg_t* msg = trace_create_message(parser->trace, TM_ERROR, token.ref);   
    trace_msg_append_costr(msg, "unexpected token format: ");
    tokenizer_trace_msg_append_token_type_name(msg, token.type);
    trace_msg_append_costr(msg, " ('");
    trace_msg_append(msg,
        srcref_ptr(token.ref),
        srcref_len(token.ref));
    trace_msg_append_costr(msg, "')");
    return par_error();
}

pa_result_t _par_set_error(parser_t* parser, token_t token, char* expected_str) {
    trace_msg_t* msg = trace_create_message(parser->trace, TM_ERROR, token.ref);
    trace_msg_append_costr(msg, "unexpected statement: ");
    tokenizer_trace_msg_append_token_type_name(msg, token.type);
    trace_msg_append_costr(msg, " ('");
    trace_msg_append(msg,
        srcref_ptr(token.ref),
        srcref_len(token.ref));
    trace_msg_append_costr(msg, "') ");
    if( expected_str != NULL ) {
        trace_msg_append(msg, expected_str, strlen(expected_str));
    }
    return par_error();
}

pa_result_t par_error_invalid_expression(parser_t* parser, token_t token, char* expected_str) {
    return _par_set_error(parser, token, expected_str);
}

pa_result_t par_error_invalid_statement(parser_t* parser, token_t token, char* expected_str) {
    return _par_set_error(parser, token, expected_str);
}
