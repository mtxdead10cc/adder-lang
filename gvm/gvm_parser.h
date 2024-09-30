#ifndef GVM_PARSER_H_
#define GVM_PARSER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include "gvm_types.h"
#include "gvm_utils.h"
#include "gvm_tokenizer.h"
#include "gvm_build_result.h"
#include "gvm_compiler_types.h"

build_result_t parser_init(parser_t* parser, char* text, size_t text_length, char* filepath);
void parser_destroy(parser_t* parser);
bool parser_is_at_end(parser_t* parser);
bool parser_advance(parser_t* parser);
token_type_t parser_current_token_type(parser_t* parser);
srcref_t parser_current_src_location(parser_t* parser);
bool parser_consume(parser_t* parser, token_type_t expected);

#endif // GVM_PARSER_H_