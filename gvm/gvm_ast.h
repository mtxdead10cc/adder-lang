#ifndef GVM_AST_H_
#define GVM_AST_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


typedef enum ast_node_type_t {
    AST_VALUE,
    AST_ARRAY,
    AST_IF,
    AST_IF_ELSE,
    AST_FOREACH,
    AST_BINOP,
    AST_UNOP,
    AST_ASSIGN,
    AST_VAR_DECL,
    AST_VAR_REF,
    AST_FUN_DECL,
    AST_FUN_CALL,
    AST_RETURN,
    AST_BREAK,
    AST_BLOCK
} ast_node_type_t;

typedef enum ast_value_type_t {
    AST_VALUE_TYPE_NONE,
    AST_VALUE_TYPE_NUMBER,
    AST_VALUE_TYPE_BOOL,
    AST_VALUE_TYPE_STRING
} ast_value_type_t;

typedef enum ast_binop_type_t {
    AST_BIN_ADD,
    AST_BIN_SUB,
    AST_BIN_MUL,
    AST_BIN_DIV,
    AST_BIN_AND,
    AST_BIN_OR,
    AST_BIN_XOR,
    AST_BIN_EQ,
    AST_BIN_LT_EQ,
    AST_BIN_GT_EQ,
    AST_BIN_LT,
    AST_BIN_GT
} ast_binop_type_t;

typedef enum ast_unop_type_t {
    AST_UN_NOT,
    AST_UN_NEG
} ast_unop_type_t;

typedef struct srcref_t {
    char*   source;
    size_t  idx_start;
    size_t  idx_end;
} srcref_t;

typedef struct ast_node_t ast_node_t;

typedef struct ast_value_t {
    ast_value_type_t type;
    union {
        float       _number;
        bool        _bool;
        srcref_t    _string;
    } u;
} ast_value_t;

typedef struct ast_array_t {
    ast_value_type_t type;
    size_t           count;
    ast_node_t**     content;
} ast_array_t;

typedef struct ast_block_t {
    size_t       count;
    ast_node_t** content;
} ast_block_t;

typedef struct ast_vardecl_t {
    srcref_t         name;
    ast_value_type_t type;
} ast_vardecl_t;

typedef struct ast_varref_t {
    srcref_t        name;
} ast_varref_t;

typedef struct ast_if_t {
    ast_node_t* cond;
    ast_node_t* iftrue;
} ast_if_t;

typedef struct ast_ifelse_t {
    ast_node_t* cond;
    ast_node_t* iftrue;
    ast_node_t* iffalse;
} ast_ifelse_t;

typedef struct ast_foreach_t {
    ast_node_t* vardecl;    // this should be an ast_vardecl_t
    ast_node_t* collection; // var-ref or array-decl
    ast_node_t* during;     // block or single instruction
} ast_foreach_t;

typedef struct ast_binop_t {
    ast_binop_type_t type;
    ast_node_t*      left;
    ast_node_t*      right;
} ast_binop_t;

typedef struct ast_unop_t {
    ast_unop_type_t  type;
    ast_node_t*      inner;
} ast_unop_t;

typedef struct ast_fundecl_t {
    ast_value_type_t rettype;
    srcref_t    name;    
    ast_node_t* args;       // block, single instruction or null
    ast_node_t* body;       // block, single instruction or null
} ast_fundecl_t;

typedef struct ast_funcall_t {
    srcref_t    name;
    ast_node_t* args;
} ast_funcall_t;

typedef struct ast_assign_t {
    ast_node_t* left_var;
    ast_node_t* right_value;
} ast_assign_t;

typedef struct ast_return_t {
    ast_node_t* result;
} ast_return_t;
 
typedef struct ast_node_t {
    ast_node_type_t     type;
    union {
        ast_value_t     n_value;
        ast_varref_t    n_varref;
        ast_array_t     n_array;
        ast_vardecl_t   n_vardecl;
        ast_block_t     n_block;
        ast_if_t        n_if;
        ast_ifelse_t    n_ifelse;
        ast_fundecl_t   n_fundecl;
        ast_binop_t     n_binop;
        ast_unop_t      n_unop;
        ast_assign_t    n_assign;
        ast_return_t    n_return;
        ast_foreach_t   n_foreach;
        ast_funcall_t   n_funcall;
    } u;
} ast_node_t;

inline static srcref_t srcref(char* text, size_t start, size_t len) {
    return (srcref_t) {
        .idx_start = start,
        .idx_end = start + len,
        .source = text
    };
}

inline static size_t srcref_len(srcref_t ref) {
    return ref.idx_end - ref.idx_start;
}

inline static char* srcref_to_str(srcref_t ref) {
    return ref.source + ref.idx_start;
}

inline static void srcref_print(srcref_t ref) {
    size_t len = srcref_len(ref);
    char buf[len + 1];
    strncpy(buf, srcref_to_str(ref), len);
    buf[len] = '\0';
    printf("%s", buf);
}

inline static bool srcref_equals(srcref_t a, srcref_t b) {
    size_t len = srcref_len(a);
    if( len != srcref_len(b) ) {
        return false;
    }
    char* a_str = srcref_to_str(a);
    char* b_str = srcref_to_str(b);
    return strncmp(a_str, b_str, len) == 0;
}

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

inline static ast_node_t* ast_fundecl( srcref_t name,
                                       ast_value_type_t return_type,
                                       ast_node_t* args,
                                       ast_node_t* body ) 
{
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_FUN_DECL,
    node->u.n_fundecl = (ast_fundecl_t) {
        .name = name,
        .rettype = return_type,
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
                                  ast_node_t* if_true ) 
{
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_IF,
    node->u.n_if = (ast_if_t) {
        .cond = cond,
        .iftrue = if_true
    };
    return node;
}

inline static ast_node_t* ast_if_else( ast_node_t* cond,
                                       ast_node_t* if_true,
                                       ast_node_t* if_false ) 
{
    ast_node_t* node = (ast_node_t*) malloc(sizeof(ast_node_t));
    node->type = AST_IF_ELSE,
    node->u.n_ifelse = (ast_ifelse_t) {
        .cond = cond,
        .iftrue = if_true,
        .iffalse = if_false
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
        case AST_IF: {
            ast_free(node->u.n_if.cond);
            ast_free(node->u.n_if.iftrue);
        } break;
        case AST_IF_ELSE: {
            ast_free(node->u.n_ifelse.cond);
            ast_free(node->u.n_ifelse.iftrue);
            ast_free(node->u.n_ifelse.iffalse);
        } break;
        case AST_FOREACH: {
            ast_free(node->u.n_foreach.collection);
            ast_free(node->u.n_foreach.vardecl);
            ast_free(node->u.n_foreach.during);
        } break;
        case AST_FUN_DECL: {
            ast_free(node->u.n_fundecl.args);
            ast_free(node->u.n_fundecl.body);
        } break;
        case AST_FUN_CALL: {
            ast_free(node->u.n_funcall.args);
        } break;
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
        case AST_IF:        return "IF";
        case AST_IF_ELSE:   return "IF_ELSE";
        case AST_FOREACH:   return "FOREACH";
        case AST_BINOP:     return "BINOP";
        case AST_UNOP:      return "UNOP";
        case AST_ASSIGN:    return "ASSIGN";
        case AST_VAR_DECL:  return "VAR_DECL";
        case AST_FUN_DECL:  return "FUN_DECL";
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
            _ast_nl(indent + 1);
            size_t count = node->u.n_array.count;
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
            _ast_nl(indent + 1);
            size_t count = node->u.n_block.count;
            for(size_t i = 0; i < count; i++) {
                _ast_dump(node->u.n_block.content[i], indent+1);
                if( i < (count - 1) ) {
                    _ast_nl(indent + 1);
                }
            }
        } break;
        case AST_IF: {
            _ast_dump(node->u.n_if.cond, indent);
            printf(" ");
            _ast_dump(node->u.n_if.iftrue, indent);
        } break;
        case AST_IF_ELSE: {
            _ast_dump(node->u.n_ifelse.cond, indent);
            printf(" ");
            _ast_dump(node->u.n_ifelse.iftrue, indent);
            printf(" ");
            _ast_dump(node->u.n_ifelse.iffalse, indent);
        } break;
        case AST_FOREACH: {
            _ast_dump(node->u.n_foreach.vardecl, indent);
            printf(" ");
            _ast_dump(node->u.n_foreach.collection, indent);
            _ast_nl(indent + 1);
            _ast_dump(node->u.n_foreach.during, indent);
        } break;
        case AST_FUN_DECL: {
            _ast_nl(indent + 1);
            printf(" name: "); srcref_print(node->u.n_fundecl.name);                                _ast_nl(indent + 1);
            printf(" type: "); printf("%s", ast_value_type_as_string(node->u.n_fundecl.rettype));   _ast_nl(indent + 1);
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