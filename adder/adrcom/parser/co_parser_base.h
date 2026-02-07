#ifndef GVM_PARSER_H_
#define GVM_PARSER_H_

#include "adrcom/parser/co_parser_types.h"

#include <adrcom/ast/co_ast.h>

#include <shared/sh_types.h>
#include <shared/sh_src.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>

bool        pa_init(parser_t* parser, arena_t* arena, src_t* source);
void        pa_destroy(parser_t* parser);

bool        pa_is_at_end(parser_t* parser);
bool        pa_advance(parser_t* parser);

token_t     pa_current_token(parser_t* parser);
token_t     pa_peek_token(parser_t* parser, int offset);

diag_t*     pa_consume(parser_t* parser, token_type_t expected);
bool        pa_advance_if(parser_t* parser, token_type_t type);
bool        pa_advance_if_not(parser_t* parser, token_type_t type);
void        pa_store_diag(parser_t* parser, ast_t* node);

bool        par_is_nothing(ast_t* res);
bool        par_is_error(ast_t* res);
bool        par_is_node(ast_t* res);

diag_t*     par_error_invalid_expression(parser_t* parser, token_t token, char* expected_str);
diag_t*     par_error_invalid_statement(parser_t* parser, token_t token, char* expected_str);
diag_t*     par_error_out_of_tokens(parser_t* parser);
diag_t*     par_error_unexpected_token_type(parser_t* parser, token_type_t expected, token_t actual);
diag_t*     par_error_invalid_token_format(parser_t* parser, token_t token);


#endif // GVM_PARSER_H_
