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
#include "gvm_cres.h"
#include "gvm_ctypes.h"

typedef enum pa_result_type_t {
    PAR_NOTHING,
    PAR_OUT_OF_TOKENS,
    PAR_AST_NODE,
    PAR_BUILD_ERROR
} pa_result_type_t;

typedef struct pa_result_t {
    pa_result_type_t    type; 
    void*               data;
} pa_result_t;

bool     pa_init(parser_t* parser, char* text, size_t text_length, char* filepath);
void     pa_destroy(parser_t* parser);
bool     pa_is_at_end(parser_t* parser);
bool     pa_advance(parser_t* parser);
token_t  pa_current_token(parser_t* parser);
token_t  pa_peek_token(parser_t* parser, int lookahead);
srcref_t pa_current_src_location(parser_t* parser);
pa_result_t pa_consume(parser_t* parser, token_type_t expected);

inline static pa_result_t par_node(ast_node_t* node) {
    return (pa_result_t) {
        .type = PAR_AST_NODE,
        .data = node
    };
}

inline static pa_result_t par_nothing(void) {
    return (pa_result_t) {
        .type = PAR_NOTHING,
        .data = NULL
    };
}

inline static pa_result_t par_out_of_tokens(void) {
    return (pa_result_t) {
        .type = PAR_OUT_OF_TOKENS,
        .data = NULL
    };
}

inline static pa_result_t par_error(cres_t* err) {
    return (pa_result_t) {
        .type = PAR_BUILD_ERROR,
        .data = err
    };
}

inline static bool par_is_nothing(pa_result_t res) {
    return res.type == PAR_NOTHING;
}

inline static bool par_is_error(pa_result_t res) {
    return res.type == PAR_BUILD_ERROR
        || res.type == PAR_OUT_OF_TOKENS;
}

inline static bool par_is_node(pa_result_t res) {
    return res.type == PAR_AST_NODE;
}

inline static ast_node_t* par_extract_node(pa_result_t res) {
    assert(res.type == PAR_AST_NODE);
    return (ast_node_t*) res.data;
}

pa_result_t pa_parse_expression(parser_t* parser);
pa_result_t pa_parse_statement(parser_t* parser);
pa_result_t pa_try_parse_fundecl(parser_t* parser);


#endif // GVM_PARSER_H_