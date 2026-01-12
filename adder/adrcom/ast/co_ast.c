#include "adrcom/ast/co_ast.h"
#include "shared/sh_json.h"

ast_t* ast_leaf(arena_t* allocator, ast_tag_t tag) {
    ast_t* node = (ast_t*) aalloc(allocator, sizeof(ast_t));
    *node = (ast_t) { 0 };
    node->tag = tag;
    return node;
}

ast_t* ast(arena_t* allocator, ast_tag_t tag, int size) {
    ast_t* node = (ast_t*) aalloc(allocator, sizeof(ast_t));
    int capacity = ast_calculate_capacity(size);
    assert(capacity >= size);
    *node = (ast_t) {
        .tag = tag,
        .size = size,
        .as.items = (ast_t**) aalloc(allocator, capacity * sizeof(ast_t*))
    };
    return node;
}


srcref_t ast_agg_srcrefs(ast_t* node) {

    srcref_t agg = { 0 };

    for(int i = 0; i < node->size; i++) {

        if(node->as.items[i]->tag == AST_UNDEFINED)
            continue;
        if(node->as.items[i]->tag == AST_SRCREF)
            agg = srcref_combine(agg, node->as.items[i]->as.srcref);
        else if(node->as.items[i]->tag == AST_STRING)
            agg = srcref_combine(agg, node->as.items[i]->as.srcref);
        else if( node->as.items[i]->size > 0 ) {
            for(int j = 0; j < node->as.items[i]->size; j++) {
                agg = srcref_combine(agg,
                    ast_agg_srcrefs(node->as.items[i]->as.items[j]));
            }
        }
    }

    return agg;
}

srcref_t ast_try_get_name(ast_t* n) {
    switch(n->tag) {
        case AST_SRCREF:    return n->as.srcref;
        case AST_VARDECL:   /* FALLTHROUGH */
        case AST_VARREF:    /* FALLTHROUGH */
        case AST_FUNCALL:   /* FALLTHROUGH */
        case AST_FUNDECL:   /* FALLTHROUGH */
        case AST_FUNDEF: {
            for(int i = 0; i < n->size; i++) {
                srcref_t srcref = ast_try_get_name(n->as.items[i]);
                if( srcref_is_valid(srcref) )
                    return srcref;
            }
        }                   /* FALLTHROUGH */
        default:            break;
    }
    return (srcref_t) { 0 };
}

ast_t* ast_try_get(ast_t* n, ast_tag_t tag) {
    if(n->tag == tag)
        return n;
    for(int i = 0; i < n->size; i++) {
        if(n->as.items[i]->tag == tag)
            return n->as.items[i];
    }
    return NULL;
}

ast_flags_t ast_try_get_flags(ast_t* n) {
    ast_t* result = ast_try_get(n, AST_FLAGS);
    if(result != NULL)
        return result->as.flags;
    return AST_FLAG_NOFLAG;
}

const char* ast_tag_to_string(ast_tag_t tag) {
    switch(tag) {
        case AST_UNDEFINED:                 return "AST_UNDEFINED";
        case AST__BEGIN_VALUES:             return "AST__BEGIN_VALUES";
        case AST_INT:                       return "AST_INT";
        case AST_FLOAT:                     return "AST_FLOAT";
        case AST_BOOL:                      return "AST_BOOL";
        case AST_CHAR:                      return "AST_CHAR";
        case AST_STRING:                    return "AST_STRING";
        case AST_SRCREF:                    return "AST_SRCREF";
        case AST_ARRAY:                     return "AST_ARRAY";
        case AST__END_VALUES:               return "AST__END_VALUES";
        case AST__BEGIN_UNARY_OPERATORS:    return "AST__BEGIN_UNARY_OPERATORS";
        case AST_UNA_NOT:                   return "AST_UNA_NOT";
        case AST_UNA_NEG:                   return "AST_UNA_NEG";
        case AST__END_UNARY_OPERATORS:      return "AST__END_UNARY_OPERATORS";
        case AST__BEGIN_BINARY_OPERATORS:   return "AST__BEGIN_BINARY_OPERATORS";
        case AST_BIN_MUL:                   return "AST_BIN_MUL";
        case AST_BIN_DIV:                   return "AST_BIN_DIV";
        case AST_BIN_MOD:                   return "AST_BIN_MOD";
        case AST_BIN_ADD:                   return "AST_BIN_ADD";
        case AST_BIN_SUB:                   return "AST_BIN_SUB";
        case AST_BIN_XOR:                   return "AST_BIN_XOR";
        case AST_BIN_LT:                    return "AST_BIN_LT";
        case AST_BIN_GT:                    return "AST_BIN_GT";
        case AST_BIN_LT_EQ:                 return "AST_BIN_LT_EQ";
        case AST_BIN_GT_EQ:                 return "AST_BIN_GT_EQ";
        case AST_BIN_EQ:                    return "AST_BIN_EQ";
        case AST_BIN_NEQ:                   return "AST_BIN_NEQ";
        case AST_BIN_OR:                    return "AST_BIN_OR";
        case AST_BIN_AND:                   return "AST_BIN_AND";
        case AST__END_BINARY_OPERATORS:     return "AST__END_BINARY_OPERATORS";
        case AST__BEGIN_HIGH_LEVEL:         return "AST__BEGIN_HIGH_LEVEL";
        case AST_TYDESCR:                   return "AST_TYDESCR";
        case AST_VARREF:                    return "AST_VARREF";
        case AST_FUNDECL:                   return "AST_FUNDECL";
        case AST_FUNCALL:                   return "AST_FUNCALL";
        case AST_FOREACH:                   return "AST_FOREACH";
        case AST_BLOCK:                     return "AST_BLOCK";
        case AST_ARGLIST:                   return "AST_ARGLIST";
        case AST_IFCHAIN:                   return "AST_IFCHAIN";
        case AST_RETURN:                    return "AST_RETURN";
        case AST_ASSIGN:                    return "AST_ASSIGN";
        case AST__END_HIGH_LEVEL:           return "AST__END_HIGH_LEVEL";
        default:                            return "<UNKNOWN AST TAG>";
    }
}

json_value_t* ast_value_to_json(ast_t* value) {

    const char* tag_name = ast_tag_to_string(value->tag);

    json_value_t* wrapper = json_object(4);
    if( wrapper == NULL ) {
        sh_log_error("ast_value_to_json: allocation failed");
        return NULL;
    }

    bool export_ok = json_object_set(wrapper,
        json_const_string("tag"),
        json_const_string(tag_name),
        true);

    if( export_ok == false ) {
        sh_log_error("ast_value_to_json: allocation failed");
        json_free(wrapper);
        return false;
    }

    switch(value->tag) {
        case AST_INT: {
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                json_number_integer(value->as.value_int),
                true);
        } break;
        case AST_BOOL: {
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                json_boolean(value->as.value_bool),
                true);
        } break;
        case AST_CHAR: {
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                json_string(&value->as.value_char, 1),
                true);
        } break;
        case AST_STRING: {
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                json_string(
                    srcref_ptr(value->as.srcref),
                    srcref_len(value->as.srcref)),
                true);
        } break;
        case AST_FLOAT: {
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                json_number_double(value->as.value_float),
                true);
        } break;
        case AST_UNDEFINED:{
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                json_null(),
                true);
        } break;
        case AST_SRCREF: {

            json_value_t* obj = json_object(4);

            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                obj,
                true);

            if( export_ok == false ) {
                json_free(obj);
                obj = NULL;
            }

            export_ok = json_object_set(obj,
                json_const_string("idx_start"),
                json_number_integer(value->as.srcref.idx_start),
                true);

            export_ok = json_object_set(obj,
                json_const_string("idx_end"),
                json_number_integer(value->as.srcref.idx_end),
                true);

            export_ok = json_object_set(obj,
                json_const_string("text"),
                json_string(
                    srcref_ptr(value->as.srcref),
                    srcref_len(value->as.srcref)),
                true);
            
        } break;
        case AST_ARRAY: {

            json_value_t* array = json_array(value->size);
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                array,
                true);

            if( export_ok == false ) {
                json_free(array);
                array = NULL;
            }

            for(int i = 0; i < value->size; i++) {
                ast_t* ast_expr = value->as.items[i];
                export_ok = export_ok && json_array_append(array,
                    ast_to_json(ast_expr), true);
            }

        } break;

        default:
            sh_log_error("ast_value_to_json: %s is not a value tag", tag_name);
            break;
    }

    if( export_ok == false ) {
        sh_log_error("ast_value_to_json: value allocation failed");
        json_free(wrapper);
        return NULL;
    }

    return wrapper;
}

json_value_t* ast_to_json(ast_t* node) {

    assert(node != NULL);

    if( node == NULL ) {
        sh_log_error("ast_to_json: ast node was null");
        return NULL;
    }

    if(node->size == 0)
        return ast_value_to_json(node);

    json_value_t* jnode = json_object(node->size);
    if( node == NULL ) {
        sh_log_error("ast_to_json: out of memory");
        return NULL;
    }

    const char* tag = ast_tag_to_string(node->tag);
    bool export_ok = json_object_set(jnode,
            json_const_string("tag"),
            json_const_string(tag),
            true);

    if( export_ok == false ) {
        sh_log_error("ast_to_json: failed to set tag");
        return NULL;
    }

    json_value_t* jnode_map = json_object(node->size);
    if( jnode_map == NULL ) {
        json_free(jnode);
        sh_log_error("ast_to_json (map): out of memory");
        return NULL;
    }

    export_ok = json_object_set(jnode,
            json_const_string("map"),
            jnode_map,
            true);

    if( export_ok == false ) {
        json_free(jnode);
        json_free(jnode_map);
        sh_log_error("ast_to_json: failed to set map");
        return NULL;
    }

    for(int i = 0; i < node->size; i++) {
        export_ok = json_object_set(jnode_map,
            json_const_string(ast_tag_to_string(node->as.items[i]->tag)),
            ast_value_to_json(node->as.items[i]),
            true);
        if( export_ok == false ) {
            json_free(jnode);
            sh_log_error("ast_to_json: failed to set"
                         " (\"%s\": <value>) pair for node"
                         " with tag %s",
                         ast_tag_to_string(node->as.items[i]->tag),
                         ast_tag_to_string(node->tag));
            return NULL;
        }
    }

    return jnode;
}

void ast_print(ast_t* node) {
    json_value_t* json = ast_to_json(node);
    char* jstr = json_dumps(json, 2);
    sh_log(jstr);
    free(jstr);
    json_free(json);
}

