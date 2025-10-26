#ifndef GVM_AST_H_
#define GVM_AST_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "adrcom/ast/co_ast_types.h"

#include <adrcom/shared/co_types.h>
#include <adrcom/shared/co_utils.h>

#include <shared/sh_utils.h>
#include <shared/sh_arena.h>
#include <shared/sh_log.h>

ast_annot_t* ast_annot(arena_t* a, srcref_t name);
void ast_annot_add_child(arena_t* a, ast_annot_t* parent, ast_annot_t* child);
srcref_t ast_srcref_from_annotation(ast_annot_t* annot);

ast_node_t* ast_tyannot(arena_t* a, ast_annot_t* type, ast_node_t* expr);

ast_node_t* ast_int(arena_t* a, int val);
ast_node_t* ast_float(arena_t* a, float val);
ast_node_t* ast_bool(arena_t* a, bool val);
ast_node_t* ast_char(arena_t* a, char val);
ast_node_t* ast_varref(arena_t* a, srcref_t name);
ast_node_t* ast_string(arena_t* a, srcref_t val);

ast_node_t* ast_block(arena_t* a);
void ast_block_add(arena_t* a, ast_node_t* block, ast_node_t* node);
ast_node_t* ast_block_with(arena_t* a, ast_node_t* content);

ast_node_t* ast_array(arena_t* a);
void ast_array_add(arena_t* a, ast_node_t* array, ast_node_t* node);

ast_node_t* ast_arglist(arena_t* a);
void ast_arglist_add(arena_t* a, ast_node_t* args, ast_node_t* node);

ast_node_t* ast_return(arena_t* a, ast_node_t* ret);

ast_node_t* ast_break(arena_t* a);

ast_node_t* ast_funexdecl(arena_t* a, srcref_t name, ast_node_t* args);
ast_node_t* ast_fundecl(arena_t* a, srcref_t name, ast_node_t* args, ast_node_t* body);
void ast_fundecl_set_exported(ast_node_t* node);
ast_node_t* ast_exported_fundecl(arena_t* a, srcref_t name, ast_node_t* args, ast_node_t* body);
ast_node_t* ast_funcall(arena_t* a, srcref_t name, ast_node_t* args );

ast_node_t* ast_if(arena_t* a, ast_node_t* cond, ast_node_t* if_true, ast_node_t* next);
bool ast_is_valid_else_block(ast_node_t* node);

ast_node_t* ast_foreach(arena_t* a, ast_node_t* vardecl, ast_node_t* collection, ast_node_t* loop_body);

ast_node_t* ast_binop(arena_t* a, ast_binop_type_t op, ast_node_t* left, ast_node_t* right);

ast_node_t* ast_unnop(arena_t* a, ast_unop_type_t op, ast_node_t* inner);

ast_node_t* ast_assign(arena_t* a, ast_node_t* left, ast_node_t* right);

srcref_t ast_try_extract_name(ast_node_t* n);

srcref_t ast_extract_srcref(ast_node_t* node);

char* ast_node_type_as_string(ast_node_type_t type);
char* ast_binop_type_as_string(ast_binop_type_t type);
char* ast_unop_type_as_string(ast_unop_type_t type);
char* ast_value_type_string(ast_value_type_t type);

void ast_dump_value(cstr_t str, ast_value_t val);
void ast_dump(ast_node_t* node);

#endif // GVM_AST_H_
