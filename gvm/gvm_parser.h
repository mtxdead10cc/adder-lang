#ifndef GVM_PARSER_H_
#define GVM_PARSER_H_

#include "gvm_types.h"

char* parser_tt_to_str(token_type_t tt);
parser_t* parser_create(char* text);
void parser_destroy(parser_t* p);

bool parser_is_at_end(parser_t* p);
bool parser_advance(parser_t* p);
bool parser_consume(parser_t* p, token_type_t tt);
bool parser_match(parser_t* p, token_type_t tt);
token_t parser_peek(parser_t* p, int lookahead);
token_t parser_current(parser_t* p);
void parser_token_as_string(parser_t* parser, token_t token, char* buffer, int max_len);
void parser_current_as_string(parser_t* parser, char* buffer, int max_len);
char* parser_get_token_char_ptr(parser_t* parser, token_t token);

#endif // GVM_PARSER_H_