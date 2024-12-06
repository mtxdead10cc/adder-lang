#ifndef GVM_PARSER_H_
#define GVM_PARSER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <stdio.h>
#include "sh_types.h"
#include "sh_utils.h"
#include "co_tokenizer.h"
#include "co_trace.h"
#include "co_types.h"
#include "co_ast.h"

typedef enum pa_result_type_t {
    PAR_NOTHING,
    PAR_AST_NODE,
    PAR_BUILD_ERROR
} pa_result_type_t;

typedef struct pa_result_t {
    pa_result_type_t    type; 
    void*               data;
    bool                group_expression; // exprs like "-(a + b)"
} pa_result_t;

pa_result_t pa_init(parser_t* parser, arena_t* arena, trace_t* trace, char* text, size_t text_length, char* filepath);
void     pa_destroy(parser_t* parser);
bool     pa_is_at_end(parser_t* parser);
bool     pa_advance(parser_t* parser);
token_t  pa_current_token(parser_t* parser);
token_t  pa_peek_token(parser_t* parser, int lookahead);
pa_result_t pa_consume(parser_t* parser, token_type_t expected);

inline static pa_result_t par_node(ast_node_t* node, srcref_t* override) {

    if( override == NULL )
        node->ref = ast_extract_srcref(node);
    else
        node->ref = *override;

    return (pa_result_t) {
        .type = PAR_AST_NODE,
        .data = node,
        .group_expression = false
    };
}

inline static pa_result_t par_nothing(void) {
    return (pa_result_t) {
        .type = PAR_NOTHING,
        .data = NULL,
        .group_expression = false
    };
}

inline static pa_result_t par_error(void) {
    return (pa_result_t) {
        .type = PAR_BUILD_ERROR,
        .data = NULL,
        .group_expression = false
    };
}

inline static bool par_is_nothing(pa_result_t res) {
    return res.type == PAR_NOTHING;
}

inline static bool par_is_error(pa_result_t res) {
    return res.type == PAR_BUILD_ERROR;
}

inline static bool par_is_node(pa_result_t res) {
    return res.type == PAR_AST_NODE;
}

inline static ast_node_t* par_extract_node(pa_result_t res) {
    assert(res.type == PAR_AST_NODE);
    return (ast_node_t*) res.data;
}

pa_result_t pa_parse_program(parser_t* parser);

#endif // GVM_PARSER_H_