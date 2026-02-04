#ifndef AST_DEBUG_H_
#define AST_DEBUG_H_

#include <stdint.h>
#include <stddef.h>

#define AST_DEBUG_CODE_INDENT_SPACES 2

typedef struct ast_t ast_t;
typedef struct cstr_t cstr_t;

typedef enum ast_dbgstyle_t {
    AST_DBG_CODE,
    AST_DBG_SEXPR
} ast_dbgstyle_t;

size_t ast_to_string(cstr_t* s, ast_t* n, ast_dbgstyle_t style);

#endif // AST_DEBUG_H_