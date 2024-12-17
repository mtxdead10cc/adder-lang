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

inline static ast_annot_t* ast_annot(arena_t* a, srcref_t name) {
    ast_annot_t* annot = (ast_annot_t*) aalloc(a, sizeof(ast_annot_t));
    annot->childcount = 0;
    annot->children = NULL;
    annot->name = name;
    return annot;
}

inline static void ast_annot_add_child(arena_t* a, ast_annot_t* parent, ast_annot_t* child) {
    parent->childcount ++;
    if( parent->children == NULL ) {
        parent->children = (ast_annot_t**) aalloc(a,
            sizeof(ast_annot_t*) * parent->childcount);
    } else {
        parent->children = (ast_annot_t**) arealloc(a,
            parent->children,
            sizeof(ast_annot_t*) * parent->childcount);
    }
    parent->children[parent->childcount - 1] = child;
}

inline static srcref_t ast_srcref_from_annotation(ast_annot_t* annot) {
    srcref_t combined = annot->name;
    for(size_t i = 0; i < annot->childcount; i++) {
        combined = srcref_combine(combined,
            ast_srcref_from_annotation(annot->children[i]));
    }
    return combined;
}

inline static ast_node_t* ast_int(arena_t* a, int val) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_VALUE;
    node->ref = (srcref_t) { 0 };
    node->u.n_value = (ast_value_t) {
        .type = AST_VALUE_INT,
        .u._int = val
    };
    return node;
}

inline static ast_node_t* ast_float(arena_t* a, float val) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_VALUE;
    node->ref = (srcref_t) { 0 };
    node->u.n_value = (ast_value_t) {
        .type = AST_VALUE_FLOAT,
        .u._float = val
    };
    return node;
}

inline static ast_node_t* ast_bool(arena_t* a, bool val) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_VALUE;
    node->ref = (srcref_t) { 0 };
    node->u.n_value = (ast_value_t) {
        .type = AST_VALUE_BOOL,
        .u._bool = val
    };
    return node;
}

inline static ast_node_t* ast_char(arena_t* a, char val) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_VALUE;
    node->ref = (srcref_t) { 0 };
    node->u.n_value = (ast_value_t) {
        .type = AST_VALUE_CHAR,
        .u._char = val
    };
    return node;
}

inline static ast_node_t* ast_varref(arena_t* a, srcref_t name) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_VAR_REF;
    node->ref = (srcref_t) { 0 };
    node->u.n_varref = (ast_varref_t) {
        .name = name
    };
    return node;
}

inline static ast_node_t* ast_tyannot(arena_t* a, ast_annot_t* type, ast_node_t* expr) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_TYANNOT;
    node->ref = (srcref_t) { 0 };
    node->u.n_tyannot = (ast_tyannot_t) {
        .type = type,
        .expr = expr
    };
    return node;
}

inline static ast_node_t* ast_block(arena_t* a) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_BLOCK;
    node->ref = (srcref_t) { 0 };
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

inline static ast_node_t* ast_block_with(arena_t* a, ast_node_t* content) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_BLOCK;
    node->ref = (srcref_t) { 0 };
    node->u.n_block = (ast_block_t) {
        .count = 0,
        .content = NULL
    };
    ast_block_add(a, node, content);
    return node;
}

inline static ast_node_t* ast_array(arena_t* a) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_ARRAY;
    node->ref = (srcref_t) { 0 };
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

inline static ast_node_t* ast_arglist(arena_t* a) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_ARGLIST;
    node->ref = (srcref_t) { 0 };
    node->u.n_args = (ast_arglist_t) {
        .count = 0,
        .content = NULL
    };
    return node;
}

inline static void ast_arglist_add(arena_t* a, ast_node_t* args, ast_node_t* node) {
    assert(args->type == AST_ARGLIST);
    if( args->u.n_args.count == 0 ) {
        assert(args->u.n_args.content == NULL);
        args->u.n_args.content = (ast_node_t**) aalloc(a, sizeof(ast_node_t*));
    } else {
        args->u.n_args.content = (ast_node_t**) arealloc(a, args->u.n_args.content, sizeof(ast_node_t*) * (args->u.n_args.count + 1));
    }
    args->u.n_args.content[args->u.n_args.count++] = node;
}

inline static ast_node_t* ast_string(arena_t* a, srcref_t val) {
    ast_node_t* char_array = ast_array(a);
    char_array->ref = val;

    // account for the fact that strings are qouted
    ptrdiff_t len = srcref_len(val) - 2;
    char*     str = srcref_ptr(val) + 1;

    for (ptrdiff_t i = 0; i < len; i++) {
        char c0 = str[i];
        char c1 = str[min(i+1, len)];
        
        srcref_t ref = srcref(val.source,
            val.idx_start + 1 + i, 1);

        // handle escape sequences

        if( c0 == '\\' ) {
            if( c1 == 'n' ) {
                c0 = 0x0A;
                i = min(i+1, len);
            } else if ( c1 == 't' ) {
                c0 = 0x09;
                i = min(i+1, len);
            } else if ( c1 == 'v' ) {
                c0 = 0x0B;
                i = min(i+1, len);
            } else if ( c1 == 'a' ) {
                c0 = 0x07;
                i = min(i+1, len);
            } else if ( c1 == '\\' ) {
                i = min(i+1, len);
            }
        }

        ast_node_t* n = ast_char(a, c0);
        n->ref = ref;
        ast_array_add(a, char_array, n);
    }

    return char_array;
}

inline static ast_node_t* ast_return(arena_t* a, ast_node_t* ret) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_RETURN;
    node->ref = (srcref_t) { 0 };
    node->u.n_return = (ast_return_t) {
        .result = ret
    };
    return node;
}

inline static ast_node_t* ast_break(arena_t* a) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_BREAK;
    node->ref = (srcref_t) { 0 };
    return node;
}

inline static ast_node_t* ast_funexdecl( arena_t* a, srcref_t name,
                                         ast_node_t* args ) 
{
    assert(args->type == AST_ARGLIST);
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_FUN_EXDECL;
    node->ref = (srcref_t) { 0 };
    node->u.n_funexdecl = (ast_funexdecl_t) {
        .name = name,
        .argspec = args
    };
    return node;
}

inline static ast_node_t* ast_fundecl( arena_t* a, srcref_t name,
                                       ast_node_t* args,
                                       ast_node_t* body ) 
{
    assert(body->type == AST_BLOCK);
    assert(args->type == AST_ARGLIST);
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_FUN_DECL;
    node->ref = (srcref_t) { 0 };
    node->u.n_fundecl = (ast_fundecl_t) {
        .name = name,
        .argspec = args,
        .body = body,
        .exported = false
    };
    return node;
}

inline static void ast_fundecl_set_exported(ast_node_t* node) {
    assert(node->type == AST_FUN_DECL);
    node->u.n_fundecl.exported = true;
}

inline static ast_node_t* ast_exported_fundecl( arena_t* a, srcref_t name,
                                       ast_node_t* args,
                                       ast_node_t* body ) 
{
    ast_node_t* node = ast_fundecl(a, name, args, body);
    ast_fundecl_set_exported(node);
    return node;
}

inline static ast_node_t* ast_funcall( arena_t* a, srcref_t name,
                                       ast_node_t* args ) 
{
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_FUN_CALL;
    node->ref = (srcref_t) { 0 };
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
    node->type = AST_IF_CHAIN;
    node->ref = (srcref_t) { 0 };
    node->u.n_if = (ast_if_t) {
        .cond = cond,
        .iftrue = if_true,
        .next = next
    };
    return node;
}

inline static bool ast_is_valid_else_block(ast_node_t* node) {
    if( node == NULL )
        return false;
    return node->type == AST_BLOCK
        && node->u.n_block.count > 0;
}

inline static ast_node_t* ast_foreach( arena_t* a, ast_node_t* vardecl,
                                       ast_node_t* collection,
                                       ast_node_t* loop_body ) 
{
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_FOREACH;
    node->ref = (srcref_t) { 0 };
    node->u.n_foreach = (ast_foreach_t) {
        .vardecl = vardecl,
        .collection = collection,
        .during = loop_body
    };
    return node;
}

inline static ast_node_t* ast_binop(arena_t* a, ast_binop_type_t op, ast_node_t* left, ast_node_t* right) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_BINOP;
    node->ref = (srcref_t) { 0 };
    node->u.n_binop = (ast_binop_t) {
        .type = op,
        .left = left,
        .right = right
    };
    return node;
}

inline static ast_node_t* ast_unnop(arena_t* a, ast_unop_type_t op, ast_node_t* inner) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_UNOP;
    node->ref = (srcref_t) { 0 };
    node->u.n_unop = (ast_unop_t) {
        .type = op,
        .inner = inner
    };
    return node;
}

inline static ast_node_t* ast_assign(arena_t* a, ast_node_t* left, ast_node_t* right) {
    ast_node_t* node = (ast_node_t*) aalloc(a, sizeof(ast_node_t));
    node->type = AST_ASSIGN;
    node->ref = (srcref_t) { 0 };
    node->u.n_assign = (ast_assign_t) {
        .left_var = left,
        .right_value = right
    };
    return node;
}

inline static srcref_t ast_try_extract_name(ast_node_t* n) {
    switch(n->type) {
        case AST_VAR_REF:       return n->u.n_varref.name;
        case AST_FUN_DECL:      return n->u.n_fundecl.name;
        case AST_FUN_EXDECL:    return n->u.n_funexdecl.name;
        case AST_TYANNOT:       return ast_try_extract_name(n->u.n_tyannot.expr);
        default:                return (srcref_t) { 0 };
    }
}

inline static srcref_t ast_extract_srcref(ast_node_t* node) {
    switch (node->type) {
        case AST_ARRAY: {
            srcref_t combined = { 0 };
            for(size_t i = 0; i < node->u.n_array.count; i++) {
                combined = srcref_combine(combined,
                    ast_extract_srcref(node->u.n_array.content[i]));
            }
            return combined;
        } break;
        case AST_IF_CHAIN: {
            srcref_t combined = { 0 };
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_if.cond));
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_if.iftrue));
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_if.next));
            return combined;
        } break;
        case AST_FOREACH: {
            srcref_t combined = { 0 };
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_foreach.vardecl));
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_foreach.collection));
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_foreach.during));
            return combined;
        } break;
        case AST_BINOP: {
            srcref_t combined = { 0 };
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_binop.left));
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_binop.right));
            return combined;
        } break;
        case AST_UNOP: {
            srcref_t combined = { 0 };
            combined = srcref_combine(combined,
                node->ref);
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_unop.inner));
            return combined;
        } break;
        case AST_ASSIGN: {
            srcref_t combined = { 0 };
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_assign.left_var));
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_assign.right_value));
            return combined;
        } break;
        case AST_TYANNOT: {
            srcref_t combined = { 0 };
            combined = srcref_combine(combined, 
                ast_srcref_from_annotation(node->u.n_tyannot.type));
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_tyannot.expr));
            return combined;
        } break;
        case AST_FUN_DECL: {
            srcref_t combined = { 0 };
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_fundecl.argspec));
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_fundecl.body));
            combined = srcref_combine(combined,
                node->u.n_funexdecl.name);
            return combined;
        } break;
        case AST_FUN_EXDECL: {
            srcref_t combined = { 0 };
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_funexdecl.argspec));
            combined = srcref_combine(combined,
                node->u.n_funexdecl.name);
            return combined;
        } break;
        case AST_FUN_CALL: {
            srcref_t combined = { 0 };
            combined = srcref_combine(combined,
                ast_extract_srcref(node->u.n_funcall.args));
            combined = srcref_combine(combined,
                node->u.n_funcall.name);
            return combined;
        } break;
        case AST_RETURN: {
            return ast_extract_srcref(node->u.n_return.result);
        } break;
        case AST_BLOCK: {
            srcref_t combined = { 0 };
            for(size_t i = 0; i < node->u.n_block.count; i++) {
                combined = srcref_combine(combined,
                    ast_extract_srcref(node->u.n_block.content[i]));
            }
            return combined;
        } break;
        case AST_ARGLIST: {
            srcref_t combined = { 0 };
            for(size_t i = 0; i < node->u.n_args.count; i++) {
                combined = srcref_combine(combined,
                    ast_extract_srcref(node->u.n_args.content[i]));
            }
            return combined;
        } break;
        default: {
            return node->ref;
        } break;      
    }
}

inline static char* ast_node_type_as_string(ast_node_type_t type) {
    switch (type) {
        case AST_VALUE:         return "VALUE";
        case AST_VAR_REF:       return "VAR_REF";
        case AST_ARRAY:         return "ARRAY";
        case AST_IF_CHAIN:      return "IF";
        case AST_FOREACH:       return "FOREACH";
        case AST_BINOP:         return "BINOP";
        case AST_UNOP:          return "UNOP";
        case AST_ASSIGN:        return "ASSIGN";
        case AST_TYANNOT:       return "AST_TYANNOT";
        case AST_FUN_DECL:      return "FUN_DECL";
        case AST_FUN_EXDECL:    return "FUN_EXDECL";
        case AST_FUN_CALL:      return "FUN_CALL";
        case AST_RETURN:        return "RETURN";
        case AST_BREAK:         return "BREAK";
        case AST_BLOCK:         return "BLOCK";
        case AST_ARGLIST:       return "AST_ARGLIST";
        default:                return "<UNKNOWN-AST-NODE-TYPE>";
    }
}

inline static char* ast_binop_type_as_string(ast_binop_type_t type) {
    switch(type) {
        case AST_BIN_ADD:   return "ADD";
        case AST_BIN_SUB:   return "SUB";
        case AST_BIN_MUL:   return "MUL";
        case AST_BIN_MOD:   return "MOD";
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

inline static char* ast_value_type_string(ast_value_type_t type) {
    switch(type) {
        case AST_VALUE_BOOL:    return LANG_TYPENAME_BOOL;
        case AST_VALUE_CHAR:    return LANG_TYPENAME_CHAR;
        case AST_VALUE_INT:     return LANG_TYPENAME_INT;
        case AST_VALUE_FLOAT:   return LANG_TYPENAME_FLOAT;
        case AST_VALUE_NONE:    return LANG_TYPENAME_VOID;
        default: return "<UNKNOWN-VALUE-TYPE>";
    }
}

inline static void ast_dump_value(ast_value_t val) {
    switch(val.type) {
        case AST_VALUE_BOOL:    printf("%s", val.u._bool ? "true" : "false");   break;
        case AST_VALUE_CHAR:    printf("'%c'", val.u._char);                    break;
        case AST_VALUE_INT:     printf("%i", val.u._int);                       break;
        case AST_VALUE_FLOAT:   printf("%f", val.u._float);                     break;
        case AST_VALUE_NONE:    printf("none");                                 break;
        default: printf("<UNKNOWN-VALUE>");                                     break;
    }
}

inline static void _ast_nl(int indent) {
    printf("\n%*s", (indent * 4), "");
}

inline static void _ast_dump_annot(ast_annot_t* a) {
    printf("(");
    srcref_print(a->name); 
    printf(" (");
    for(size_t i = 0; i < a->childcount; i++) {
        _ast_dump_annot(a->children[i]);
    }
    printf("))");
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
        case AST_TYANNOT: {
            _ast_dump(node->u.n_tyannot.expr, indent);
            printf(": ");
            _ast_dump_annot(node->u.n_tyannot.type);
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
        case AST_ARGLIST: {
            size_t count = node->u.n_args.count;
            if( count > 0 )
                _ast_nl(indent + 1);
            for(size_t i = 0; i < count; i++) {
                _ast_dump(node->u.n_args.content[i], indent+1);
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
        case AST_FUN_DECL: {
            srcref_print(node->u.n_fundecl.name);
            _ast_dump(node->u.n_fundecl.argspec, indent + 1);
            printf(" body: "); _ast_dump(node->u.n_fundecl.body, indent + 2); _ast_nl(indent);
        } break;
        case AST_FUN_EXDECL: {
            srcref_print(node->u.n_funexdecl.name);
            _ast_dump(node->u.n_funexdecl.argspec, indent + 1);
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
