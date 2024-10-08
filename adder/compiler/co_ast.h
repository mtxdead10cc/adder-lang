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

// TODO: replace malloc / free ast node with areana allocator

inline static ast_node_t* ast_number(float val) {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_VALUE,
    node->u.n_value = (ast_value_t) {
        .type = AST_VALUE_TYPE_NUMBER,
        .u._number = val
    };
    return node;
}

inline static ast_node_t* ast_bool(bool val) {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_VALUE,
    node->u.n_value = (ast_value_t) {
        .type = AST_VALUE_TYPE_BOOL,
        .u._bool = val
    };
    return node;
}

inline static ast_node_t* ast_string(srcref_t val) {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_VALUE,
    node->u.n_value = (ast_value_t) {
        .type = AST_VALUE_TYPE_STRING,
        .u._string = val
    };
    return node;
}

inline static ast_node_t* ast_varref(srcref_t name) {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_VAR_REF,
    node->u.n_varref = (ast_varref_t) {
        .name = name
    };
    return node;
}

inline static ast_node_t* ast_vardecl(srcref_t name, ast_value_type_t type) {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_VAR_DECL,
    node->u.n_vardecl = (ast_vardecl_t) {
        .type = type,
        .name = name
    };
    return node;
}

inline static ast_node_t* ast_block() {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_BLOCK,
    node->u.n_block = (ast_block_t) {
        .count = 0,
        .content = NULL
    };
    return node;
}

inline static void ast_block_add(ast_node_t* block, ast_node_t* node) {
    assert(block->type == AST_BLOCK);
    if( block->u.n_block.count == 0 ) {
        assert(block->u.n_block.content == NULL);
        block->u.n_block.content = (ast_node_t**) malloc(sizeof(ast_node_t*));
    } else {
        block->u.n_block.content = (ast_node_t**) realloc(block->u.n_block.content, sizeof(ast_node_t*) * (block->u.n_block.count + 1));
    }
    block->u.n_block.content[block->u.n_block.count++] = node;
}

inline static ast_node_t* ast_array() {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_ARRAY,
    node->u.n_array = (ast_array_t) {
        .count = 0,
        .content = NULL
    };
    return node;
}

inline static void ast_array_add(ast_node_t* array, ast_node_t* node) {
    assert(array->type == AST_ARRAY);
    if( array->u.n_array.count == 0 ) {
        assert(array->u.n_array.content == NULL);
        array->u.n_array.content = (ast_node_t**) malloc(sizeof(ast_node_t*));
    } else {
        array->u.n_array.content = (ast_node_t**) realloc(array->u.n_array.content, sizeof(ast_node_t*) * (array->u.n_array.count + 1));
    }
    array->u.n_array.content[array->u.n_array.count++] = node;
}

inline static ast_node_t* ast_return(ast_node_t* ret) {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_RETURN,
    node->u.n_return = (ast_return_t) {
        .result = ret
    };
    return node;
}

inline static ast_node_t* ast_break() {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_BREAK;
    return node;
}

inline static ast_node_t* ast_funsign( srcref_t name,
                                       ast_value_type_t return_type ) 
{
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_FUN_SIGN,
    node->u.n_funsign = (ast_funsign_t) {
        .name = name,
        .rettype = return_type
    };
    return node;
}

inline static ast_node_t* ast_fundecl( ast_node_t* funsign,
                                       ast_node_t* args,
                                       ast_node_t* body ) 
{
    assert(funsign->type == AST_FUN_SIGN);
    assert(args->type == AST_BLOCK);
    assert(body->type == AST_BLOCK);
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_FUN_DECL,
    node->u.n_fundecl = (ast_fundecl_t) {
        .funsign = funsign,
        .args = args,
        .body = body
    };
    return node;
}

inline static ast_node_t* ast_funcall( srcref_t name,
                                       ast_node_t* args ) 
{
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_FUN_CALL,
    node->u.n_funcall = (ast_funcall_t) {
        .name = name,
        .args = args,
    };
    return node;
}

inline static ast_node_t* ast_if( ast_node_t* cond,
                                  ast_node_t* if_true,
                                  ast_node_t* next ) 
{
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_IF_CHAIN,
    node->u.n_if = (ast_if_t) {
        .cond = cond,
        .iftrue = if_true,
        .next = next
    };
    return node;
}


inline static ast_node_t* ast_foreach( ast_node_t* vardecl,
                                       ast_node_t* collection,
                                       ast_node_t* loop_body ) 
{
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_FOREACH,
    node->u.n_foreach = (ast_foreach_t) {
        .vardecl = vardecl,
        .collection = collection,
        .during = loop_body
    };
    return node;
}

inline static ast_node_t* ast_binop(ast_binop_type_t op, ast_node_t* left, ast_node_t* right) {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_BINOP,
    node->u.n_binop = (ast_binop_t) {
        .type = op,
        .left = left,
        .right = right
    };
    return node;
}

inline static ast_node_t* ast_unnop(ast_unop_type_t op, ast_node_t* inner) {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_UNOP,
    node->u.n_unop = (ast_unop_t) {
        .type = op,
        .inner = inner
    };
    return node;
}

inline static ast_node_t* ast_assign(ast_node_t* left, ast_node_t* right) {
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_ASSIGN,
    node->u.n_assign = (ast_assign_t) {
        .left_var = left,
        .right_value = right
    };
    return node;
}


inline static void ast_free(ast_node_t* node) {

    if( node == NULL )
        return;

    switch(node->type) {
        case AST_BINOP: {
            ast_free(node->u.n_binop.left);
            ast_free(node->u.n_binop.right);
        } break;
        case AST_UNOP: {
            ast_free(node->u.n_unop.inner);
        } break;
        case AST_ASSIGN: {
            ast_free(node->u.n_assign.left_var);
            ast_free(node->u.n_assign.right_value);
        } break;
        case AST_ARRAY: {
            size_t count = node->u.n_array.count;
            for(size_t i = 0; i < count; i++) {
                ast_free(node->u.n_array.content[i]);
            }
            if( node->u.n_array.content != NULL ) {
                free(node->u.n_array.content);
            }
            node->u.n_array.content = NULL;
            node->u.n_array.count = 0;
        } break;
        case AST_RETURN: {
            ast_free(node->u.n_return.result);
        } break;
        case AST_BLOCK: {
            size_t count = node->u.n_block.count;
            for(size_t i = 0; i < count; i++) {
                ast_free(node->u.n_block.content[i]);
            }
            if( node->u.n_block.content != NULL ) {
                free(node->u.n_block.content);
            }
            node->u.n_block.content = NULL;
            node->u.n_block.count = 0;
        } break;
        case AST_IF_CHAIN: {
            ast_free(node->u.n_if.cond);
            ast_free(node->u.n_if.iftrue);
            ast_free(node->u.n_if.next);
        } break;
        case AST_FOREACH: {
            ast_free(node->u.n_foreach.collection);
            ast_free(node->u.n_foreach.vardecl);
            ast_free(node->u.n_foreach.during);
        } break;
        case AST_FUN_DECL: {
            ast_free(node->u.n_fundecl.funsign);
            ast_free(node->u.n_fundecl.args);
            ast_free(node->u.n_fundecl.body);
        } break;
        case AST_FUN_CALL: {
            ast_free(node->u.n_funcall.args);
        } break;
        case AST_FUN_SIGN:
        case AST_VAR_REF:
        case AST_VALUE:
        case AST_VAR_DECL:
        case AST_BREAK: {
            /* nothing to free */
        } break;
    }
    
    // free the node itself
    free(node);
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

inline static char* ast_value_type_as_string(ast_value_type_t type) {
    switch(type) {
        case AST_VALUE_TYPE_NONE:       return "NONE";
        case AST_VALUE_TYPE_NUMBER:     return "NUMBER";
        case AST_VALUE_TYPE_BOOL:       return "BOOL";
        case AST_VALUE_TYPE_STRING:     return "STRING";
        default: return "<UNKNOWN-AST-VALUE_TYPE>";
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

inline static void ast_dump_value(ast_value_t val) {
    switch(val.type) {
        case AST_VALUE_TYPE_NONE:   printf("-none-"); break;
        case AST_VALUE_TYPE_NUMBER: printf("%f", val.u._number); break;
        case AST_VALUE_TYPE_BOOL:   printf("%s", val.u._bool ? "true" : "false"); break;
        case AST_VALUE_TYPE_STRING: srcref_print(val.u._string); break;
        default: printf("-unk-"); break;
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
            printf("%s ", ast_value_type_as_string(node->u.n_value.type));
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
            printf("%s ", ast_value_type_as_string(node->u.n_vardecl.type));
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
            srcref_print(node->u.n_funsign.name);
            printf(" ");
            printf("%s", ast_value_type_as_string(node->u.n_funsign.rettype)); 
        } break;
        case AST_FUN_DECL: {
            _ast_dump(node->u.n_fundecl.funsign, indent + 1);                                       _ast_nl(indent + 1);              
            printf(" args: "); _ast_dump(node->u.n_fundecl.args, indent + 2);                       _ast_nl(indent + 1);
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