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

bool pa_init(parser_t* parser, arena_t* arena, src_t* source) {
    
    if( tokens_init(&parser->collection, 16) == false ) {
        sh_log_error("pa_init: out of memory");
        return false;
    }

    if( source == NULL ) {
        sh_log_error("the input text buffer pointer was null.");
        return false;
    }

    tokenizer_args_t args = (tokenizer_args_t) {
        .source = source,
        .include_comments = false,
        .include_spaces = false
    };

    if( tokenizer_analyze(&parser->collection, &args) == false ) {
        tokens_destroy(&parser->collection);
        parser->collection.count = 0;
        sh_log_error("the input text buffer pointer was null.");
        return false;
    }

    parser->arena = arena;
    parser->cursor = 0;
    return true;
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

token_t pa_peek_token(parser_t* parser, int offset) {

    int total = parser->collection.count;
    int processed = parser->cursor;
    int remaining = total - processed;

    if( processed + offset < 0 )
        offset = 0;
    else if( remaining < offset )
        offset = remaining;

    return parser->collection.tokens[parser->cursor + offset];
}

diag_t* pa_consume(parser_t* parser, token_type_t expected) {
    if( pa_is_at_end(parser) ) {
        return par_error_out_of_tokens(parser);
    }
    token_t actual = pa_current_token(parser);
    diag_t* error = NULL;
    if( expected != actual.type ) {
        error = par_error_unexpected_token_type(parser, expected, actual);
    }
    pa_advance(parser); // do not check eof here
    return error;
}

bool par_is_nothing(ast_t* res) {
    return res == NULL;
}

bool par_is_error(ast_t* res) {
    if(res == NULL)
        return false;
    if(res->diagnostics == NULL)
        return false;
    return res->diagnostics->kind == DIAG_ERROR;
}

bool par_is_node(ast_t* res) {
    return res != NULL;
}

diag_t* par_error_out_of_tokens(parser_t* parser) {
    diag_t* diag = diag_error(parser->arena,
        diag_phrase(_UNEXPECTED, _END, _OF, _TOKEN, _STREAM),
        diag_refloc(pa_peek_token(parser, 0).ref));
    return diag;
}

diag_t* par_error_unexpected_token_type(parser_t* parser, token_type_t expected, token_t actual) {

    (void)(parser);

    diag_t* diag = diag_error(parser->arena,
        diag_phrase(_UNEXPECTED, _TOKEN),
        diag_str(" at "), diag_refloc(actual.ref),
        diag_str("\n  expected "), diag_str(token_get_type_name(expected) + 3),
        diag_str("\n  got      "), diag_refstr(actual.ref));
    return diag;
}

diag_t* par_error_invalid_token_format(parser_t* parser, token_t token) {
    
    (void)(parser);

    diag_t* diag = diag_error(parser->arena,
        diag_phrase(_UNEXPECTED, _TOKEN, _FORMAT),
        diag_str(" at "), diag_refloc(token.ref),
        diag_str("\n  "),
        diag_str(token_get_type_name(token.type) + 3),
        diag_str(" ("), diag_refstr(token.ref), diag_str(")"));
    return diag;
}

diag_t* _par_set_error(parser_t* parser, token_t token, char* expected_str) {

    (void)(parser);

    if(expected_str != NULL) {

        diag_t* diag = diag_error(parser->arena,
            diag_phrase(_UNEXPECTED, _STATEMENT),
            diag_str("at "), diag_refloc(token.ref),
            diag_str("\n  "), diag_str(token_get_type_name(token.type) + 3),
            diag_str(" ("), diag_refstr(token.ref), diag_str(")"),
            diag_str("\n  expected "), diag_str(expected_str));

        return diag;
    }

    diag_t* diag = diag_error(parser->arena,
        diag_phrase(_UNEXPECTED, _STATEMENT),
        diag_str("at "), diag_refloc(token.ref),
        diag_str("\n  "), diag_str(token_get_type_name(token.type) + 3),
        diag_str(" ("), diag_refstr(token.ref), diag_str(")"));
        
    return diag;
}

diag_t* par_error_invalid_expression(parser_t* parser, token_t token, char* expected_str) {
    return _par_set_error(parser, token, expected_str);
}

diag_t* par_error_invalid_statement(parser_t* parser, token_t token, char* expected_str) {
    return _par_set_error(parser, token, expected_str);
}
