#ifndef GVM_PARSER_H_
#define GVM_PARSER_H_

#include "adrcom/parser/co_parser_types.h"

#include <adrcom/ast/co_ast.h>

#include <shared/sh_types.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>

pa_result_t pa_init(parser_t* parser, arena_t* arena, trace_t* trace, char* text, size_t text_length, char* filepath);
void        pa_destroy(parser_t* parser);

bool        pa_is_at_end(parser_t* parser);
bool        pa_advance(parser_t* parser);

token_t     pa_current_token(parser_t* parser);
token_t     pa_peek_token(parser_t* parser, int lookahead);

pa_result_t pa_consume(parser_t* parser, token_type_t expected);
bool        pa_advance_if(parser_t* parser, token_type_t type);
bool        pa_advance_if_not(parser_t* parser, token_type_t type);

pa_result_t par_node(ast_t* node);
pa_result_t par_nothing(void);
pa_result_t par_error(void);

bool        par_is_nothing(pa_result_t res);
bool        par_is_error(pa_result_t res);
bool        par_is_node(pa_result_t res);

ast_t*      par_extract_node(pa_result_t res);

pa_result_t par_error_invalid_expression(parser_t* parser, token_t token, char* expected_str);
pa_result_t par_error_invalid_statement(parser_t* parser, token_t token, char* expected_str);
pa_result_t par_error_out_of_tokens(parser_t* parser);
pa_result_t par_error_unexpected_token_type(parser_t* parser, token_type_t expected, token_t actual);
pa_result_t par_error_invalid_token_format(parser_t* parser, token_t token);


#endif // GVM_PARSER_H_
