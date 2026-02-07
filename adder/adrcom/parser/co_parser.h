#ifndef GVM_LANGUAGE_PARSER_H_
#define GVM_LANGUAGE_PARSER_H_

#include "adrcom/parser/co_parser_types.h"
#include "adrcom/parser/co_parser_base.h"

ast_t* pa_parse_program(parser_t* parser);

#endif // GVM_LANGUAGE_PARSER_H_