#ifndef AST_JSON_H_
#define AST_JSON_H_

#include "adrcom/ast/co_ast.h"

typedef src_t* (*mksrc_fn_t)(void* user, char* path, size_t pathlen);

void          ast_print_json(ast_t* node);

typedef struct json_value_t json_value_t;

json_value_t* ast_to_json(arena_t* ator, ast_t* node);
ast_t*        ast_from_json(arena_t* ator, json_value_t* json, mksrc_fn_t srcmaker, void* user);

#endif // AST_JSON_H_