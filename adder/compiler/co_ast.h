#ifndef GVM_AST_H_
#define GVM_AST_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "sh_utils.h"
#include "co_types.h"
#include "co_utils.h"
#include "sh_arena.h"
#include "co_typing.h"

#define LANG_TYPENAME_NONE      "none"
#define LANG_TYPENAME_BOOL      "bool"
#define LANG_TYPENAME_FLOAT     "float"
#define LANG_TYPENAME_INT       "int"
#define LANG_TYPENAME_STRING    "string"
#define LANG_TYPENAME_ARRAY     "array"
#define LANG_TYPENAME_CHAR      "char"

inline static ast_node_t* ast_int(arena_t* a, int val) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_VALUE,
    node->u.n_value = (ast_value_t) {
        .type = AST_VALUE_INT,
        .u._int = val
    };
    return node;
}

inline static ast_node_t* ast_float(arena_t* a, float val) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_VALUE,
    node->u.n_value = (ast_value_t) {
        .type = AST_VALUE_FLOAT,
        .u._float = val
    };
    return node;
}

inline static ast_node_t* ast_bool(arena_t* a, bool val) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_VALUE,
    node->u.n_value = (ast_value_t) {
        .type = AST_VALUE_BOOL,
        .u._bool = val
    };
    return node;
}

inline static ast_node_t* ast_string(arena_t* a, srcref_t val) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_VALUE,
    node->u.n_value = (ast_value_t) {
        .type = AST_VALUE_STRING,
        .u._string = val
    };
    return node;
}

inline static ast_node_t* ast_varref(arena_t* a, srcref_t name) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_VAR_REF,
    node->u.n_varref = (ast_varref_t) {
        .name = name
    };
    return node;
}

inline static ast_node_t* ast_vardecl(arena_t* a, srcref_t name, srcref_t type) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_VAR_DECL,
    node->u.n_vardecl = (ast_vardecl_t) {
        .type = type,
        .name = name
    };
    return node;
}

inline static ast_node_t* ast_block(arena_t* a) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_BLOCK,
    node->u.n_block = (ast_block_t) {
        .count = 0,
        .content = NULL
    };
    return node;
}

inline static void ast_block_add(arena_t* a, ast_node_t* block, ast_node_t* node) {
    assert(block->type == AST_BLOCK);
    if( block->u.n_block.count == 0 ) {
        assert(block->u.n_block.content == NULL);
        block->u.n_block.content = (ast_node_t**) aalloc(a, sizeof(ast_node_t*));
    } else {
        block->u.n_block.content = (ast_node_t**) arealloc(a, block->u.n_block.content, sizeof(ast_node_t*) * (block->u.n_block.count + 1));
    }
    block->u.n_block.content[block->u.n_block.count++] = node;
}

inline static ast_node_t* ast_array(arena_t* a) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_ARRAY,
    node->u.n_array = (ast_array_t) {
        .count = 0,
        .content = NULL
    };
    return node;
}

inline static void ast_array_add(arena_t* a, ast_node_t* array, ast_node_t* node) {
    assert(array->type == AST_ARRAY);
    if( array->u.n_array.count == 0 ) {
        assert(array->u.n_array.content == NULL);
        array->u.n_array.content = (ast_node_t**) aalloc(a, sizeof(ast_node_t*));
    } else {
        array->u.n_array.content = (ast_node_t**) arealloc(a, array->u.n_array.content, sizeof(ast_node_t*) * (array->u.n_array.count + 1));
    }
    array->u.n_array.content[array->u.n_array.count++] = node;
}

inline static ast_node_t* ast_return(arena_t* a, ast_node_t* ret) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_RETURN,
    node->u.n_return = (ast_return_t) {
        .result = ret
    };
    return node;
}

inline static ast_node_t* ast_break(arena_t* a) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_BREAK;
    return node;
}

inline static ast_node_t* ast_funsign( arena_t* a, srcref_t name,
                                       ast_node_t* argspec,
                                       ast_decl_type_t decltype,
                                       srcref_t return_type ) 
{
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_FUN_SIGN,
    node->u.n_funsign = (ast_funsign_t) {
        .name = name,
        .argspec = argspec,
        .decltype = decltype,
        .return_type = return_type
    };
    return node;
}

inline static ast_node_t* ast_fundecl( arena_t* a, ast_node_t* funsign,
                                       ast_node_t* body ) 
{
    assert(funsign->type == AST_FUN_SIGN);
    assert(body->type == AST_BLOCK);
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_FUN_DECL,
    node->u.n_fundecl = (ast_fundecl_t) {
        .funsign = funsign,
        .body = body
    };
    return node;
}

inline static ast_node_t* ast_funcall( arena_t* a, srcref_t name,
                                       ast_node_t* args ) 
{
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_FUN_CALL,
    node->u.n_funcall = (ast_funcall_t) {
        .name = name,
        .args = args,
    };
    return node;
}

inline static ast_node_t* ast_if( arena_t* a, ast_node_t* cond,
                                  ast_node_t* if_true,
                                  ast_node_t* next ) 
{
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_IF_CHAIN,
    node->u.n_if = (ast_if_t) {
        .cond = cond,
        .iftrue = if_true,
        .next = next
    };
    return node;
}


inline static ast_node_t* ast_foreach( arena_t* a, ast_node_t* vardecl,
                                       ast_node_t* collection,
                                       ast_node_t* loop_body ) 
{
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_FOREACH,
    node->u.n_foreach = (ast_foreach_t) {
        .vardecl = vardecl,
        .collection = collection,
        .during = loop_body
    };
    return node;
}

inline static ast_node_t* ast_binop(arena_t* a, ast_binop_type_t op, ast_node_t* left, ast_node_t* right) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_BINOP,
    node->u.n_binop = (ast_binop_t) {
        .type = op,
        .left = left,
        .right = right
    };
    return node;
}

inline static ast_node_t* ast_unnop(arena_t* a, ast_unop_type_t op, ast_node_t* inner) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_UNOP,
    node->u.n_unop = (ast_unop_t) {
        .type = op,
        .inner = inner
    };
    return node;
}

inline static ast_node_t* ast_assign(arena_t* a, ast_node_t* left, ast_node_t* right) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_ASSIGN,
    node->u.n_assign = (ast_assign_t) {
        .left_var = left,
        .right_value = right
    };
    return node;
}

inline static char* ast_node_type_as_string(ast_node_type_t type) {
    switch (type) {
        case AST_VALUE:     return "VALUE";
        case AST_VAR_REF:   return "VAR_REF";
        case AST_ARRAY:     return "ARRAY";
        case AST_IF_CHAIN:  return "IF";
        case AST_FOREACH:   return "FOREACH";
        case AST_BINOP:     return "BINOP";
        case AST_UNOP:      return "UNOP";
        case AST_ASSIGN:    return "ASSIGN";
        case AST_VAR_DECL:  return "VAR_DECL";
        case AST_FUN_DECL:  return "FUN_DECL";
        case AST_FUN_SIGN:  return "FUN_SIGN";
        case AST_FUN_CALL:  return "FUN_CALL";
        case AST_RETURN:    return "RETURN";
        case AST_BREAK:     return "BREAK";
        case AST_BLOCK:     return "BLOCK";
        default:            return "<UNKNOWN-AST-NODE-TYPE>";
    }
}

inline static char* ast_binop_type_as_string(ast_binop_type_t type) {
    switch(type) {
        case AST_BIN_ADD:   return "ADD";
        case AST_BIN_SUB:   return "SUB";
        case AST_BIN_MUL:   return "MUL";
        case AST_BIN_DIV:   return "DIV";
        case AST_BIN_AND:   return "AND";
        case AST_BIN_OR:    return "OR";
        case AST_BIN_XOR:   return "XOR";
        case AST_BIN_EQ:    return "EQ";
        case AST_BIN_NEQ:    return "NEQ";
        case AST_BIN_LT_EQ: return "LT_EQ";
        case AST_BIN_GT_EQ: return "GT_EQ";
        case AST_BIN_LT:    return "LT";
        case AST_BIN_GT:    return "GT";
        default: return "<UNKNOWN-BINOP-TYPE>";
    }
}

inline static char* ast_unop_type_as_string(ast_unop_type_t type) {
    switch(type) {
        case AST_UN_NOT: return "NOT";
        case AST_UN_NEG: return "NEG";
        default: return "<UNKNOWN-UNOP-TYPE>";
    }
}

inline static char* ast_decl_type_string(ast_decl_type_t type) {
    switch(type) {
        case AST_FUNSIGN_EXTERN: return "EXTERN";
        case AST_FUNSIGN_INTERN: return "INTERN";
        default: return "<UNKNOWN-FUNSIGN-TYPE>";
    }
}

inline static char* ast_value_type_string(ast_value_type_t type) {
    switch(type) {
        case AST_VALUE_BOOL:    return LANG_TYPENAME_BOOL;
        case AST_VALUE_CHAR:    return LANG_TYPENAME_CHAR;
        case AST_VALUE_INT:     return LANG_TYPENAME_INT;
        case AST_VALUE_FLOAT:   return LANG_TYPENAME_FLOAT;
        case AST_VALUE_STRING:  return LANG_TYPENAME_STRING;
        case AST_VALUE_NONE:    return LANG_TYPENAME_NONE;
        default: return "<UNKNOWN-VALUE-TYPE>";
    }
}

inline static void ast_dump_value(ast_value_t val) {
    switch(val.type) {
        case AST_VALUE_BOOL:    printf("%s", val.u._bool ? "true" : "false");   break;
        case AST_VALUE_CHAR:    printf("'%c'", val.u._char);                    break;
        case AST_VALUE_INT:     printf("%i", val.u._int);                       break;
        case AST_VALUE_FLOAT:   printf("%f", val.u._float);                     break;
        case AST_VALUE_STRING:  srcref_print(val.u._string);                    break;
        case AST_VALUE_NONE:    printf("none");                                 break;
        default: printf("<UNKNOWN-VALUE>");                                     break;
    }
}

inline static void _ast_nl(int indent) {
    printf("\n%*s", (indent * 4), "");
}

inline static void _ast_dump(ast_node_t* node, int indent) {
    printf("[");
    printf("%s ", ast_node_type_as_string(node->type));
    switch(node->type) {
        case AST_VAR_REF: {
            srcref_print(node->u.n_varref.name);
        } break;
        case AST_VALUE: {
            printf("%s ", ast_value_type_string(node->u.n_value.type));
            ast_dump_value(node->u.n_value);
        } break;
        case AST_BINOP: {
            printf("%s ", ast_binop_type_as_string(node->u.n_binop.type));
            _ast_dump(node->u.n_binop.left, indent);
            printf(" ");
            _ast_dump(node->u.n_binop.right, indent);
        } break;
        case AST_UNOP: {
            printf("%s ", ast_unop_type_as_string(node->u.n_unop.type));
            _ast_dump(node->u.n_unop.inner, indent);
        } break;
        case AST_ASSIGN: {
            _ast_dump(node->u.n_assign.left_var, indent);
            printf(" ");
            _ast_dump(node->u.n_assign.right_value, indent);
        } break;
        case AST_ARRAY: {
            size_t count = node->u.n_array.count;
            if(count == 0)
                break;
            _ast_nl(indent + 1);
            for(size_t i = 0; i < count; i++) {
                _ast_dump(node->u.n_array.content[i], indent + 1);
                if( i < (count - 1) ) {
                    _ast_nl(indent + 1);
                }
            }
        } break;
        case AST_RETURN: {
            _ast_dump(node->u.n_return.result, indent);
        } break;
        case AST_VAR_DECL: {
            srcref_print(node->u.n_vardecl.type);
            printf(" ");
            srcref_print(node->u.n_vardecl.name);
        } break;
        case AST_BREAK: {
            /* nothing */
        } break;
        case AST_BLOCK: {
            size_t count = node->u.n_block.count;
            if( count > 0 )
                _ast_nl(indent + 1);
            for(size_t i = 0; i < count; i++) {
                _ast_dump(node->u.n_block.content[i], indent+1);
                if( i < (count - 1) ) {
                    _ast_nl(indent + 1);
                }
            }
        } break;
        case AST_IF_CHAIN: {
            _ast_dump(node->u.n_if.cond, indent);
            printf(" ");
            _ast_dump(node->u.n_if.iftrue, indent);
        } break;
        case AST_FOREACH: {
            _ast_dump(node->u.n_foreach.vardecl, indent);
            printf(" ");
            _ast_dump(node->u.n_foreach.collection, indent);
            _ast_nl(indent + 1);
            _ast_dump(node->u.n_foreach.during, indent);
        } break;
        case AST_FUN_SIGN: {
            printf("%s ", ast_decl_type_string(node->u.n_funsign.decltype));
            srcref_print(node->u.n_funsign.name);
            _ast_dump(node->u.n_funsign.argspec, indent + 1);
            printf(" -> ");
            srcref_print(node->u.n_funsign.return_type);
        } break;
        case AST_FUN_DECL: {
            _ast_dump(node->u.n_fundecl.funsign, indent + 1);                                       _ast_nl(indent + 1);
            printf(" body: "); _ast_dump(node->u.n_fundecl.body, indent + 2);                       _ast_nl(indent);
        } break;
        case AST_FUN_CALL: {
            srcref_print(node->u.n_funcall.name);
            printf(" ");
            _ast_dump(node->u.n_funcall.args, indent);
        } break;
    }
    printf("]");
}

inline static void ast_dump(ast_node_t* node) {
    _ast_dump(node, 0);
    printf("\n");
}

#endif // GVM_AST_H_