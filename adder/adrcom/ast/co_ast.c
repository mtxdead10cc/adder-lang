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
    *node = (ast_t) {
        .tag = tag,
        .size = size,
        .as.items = (ast_t**) aalloc(allocator,
            ast_calculate_capacity(size) * sizeof(ast_t*))
    };
    return node;
}


srcref_t _ast_agg_srcrefs(ast_t* node, int d) {

    assert(d < 100);

    if(node->tag == AST_SYMBOL)
        return node->as.srcref;

    if(node->tag == AST_STRING)
        return node->as.srcref;

    if(node->size == 0)
        return (srcref_t) {0};

    srcref_t agg = { 0 };

    for(int i = 0; i < node->size; i++) {
        assert(node->as.items[i] != node);
        agg = srcref_combine(agg,
            _ast_agg_srcrefs(node->as.items[i], d+1));
    }

    return agg;
}

srcref_t ast_agg_srcrefs(ast_t* node) {
    return _ast_agg_srcrefs(node, 0);
}

srcref_t ast_try_get_name(ast_t* n) {
    switch(n->tag) {
        case AST_SYMBOL:    return n->as.srcref;
        case AST_VARDECL:   /* FALLTHROUGH */
        case AST_VARREF:    /* FALLTHROUGH */
        case AST_FUNCALL:   /* FALLTHROUGH */
        case AST_FUNSIGN:   /* FALLTHROUGH */
        case AST_FUNDEFN: {
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


const char* ast_tag_to_string(ast_tag_t tag) {
    switch(tag) {
        case AST_UNDEFINED:                 return "AST_UNDEFINED";
        case AST__BEGIN_VALUES:             return "AST__BEGIN_VALUES";
        case AST_INT:                       return "AST_INT";
        case AST_FLOAT:                     return "AST_FLOAT";
        case AST_BOOL:                      return "AST_BOOL";
        case AST_CHAR:                      return "AST_CHAR";
        case AST_STRING:                    return "AST_STRING";
        case AST_SYMBOL:                    return "AST_SYMBOL";
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
        case AST_VARDECL:                   return "AST_VARDECL";
        case AST_FUNDEFN:                   return "AST_FUNDEFN";
        case AST_FUNSIGN:                   return "AST_FUNSIGN";
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

json_value_t* json_value_wrapper(arena_t* ator, const char* tag_name, json_value_t* value) {

    if(ator == NULL || value == NULL)
        return NULL;

    json_value_t* wrapper = json_object(ator, 2);
    json_value_t* wrptagkey = json_const_string(ator, "tag");
    json_value_t* wrptagname = json_const_string(ator, tag_name);
    json_value_t* valuekey = json_const_string(ator, "value");

    if( json_object_set(wrapper, wrptagkey, wrptagname) == false ) {
        sh_log_error("ast_value_wrapper: allocation/insert key failed");
        return NULL;
    }

    if( json_object_set(wrapper, valuekey, value) == false ) {
        sh_log_error("ast_value_wrapper: allocation/insert value failed");
        return NULL;
    }

    return wrapper;
}

json_value_t* ast_to_json(arena_t* ator, ast_t* value) {

    if(ator == NULL || value == NULL)
        return NULL;

    const char* tag_name = ast_tag_to_string(value->tag);

    switch(value->tag) {
        case AST_INT:    return json_value_wrapper(ator, tag_name,
            json_number_integer(ator, value->as.value_int));
        case AST_BOOL:   return json_value_wrapper(ator, tag_name,
            json_boolean(ator, value->as.value_bool));
        case AST_CHAR:   return json_value_wrapper(ator, tag_name,
            json_string(ator, &value->as.value_char, 1));
        case AST_STRING: return json_value_wrapper(ator, tag_name,
            json_string(ator, 
                srcref_ptr(value->as.srcref),
                srcref_len(value->as.srcref)));
        case AST_FLOAT: return json_value_wrapper(ator, tag_name,
            json_number_double(ator, value->as.value_float));
        case AST_UNDEFINED: return json_value_wrapper(ator, tag_name,
            json_null(ator));
        case AST_SYMBOL: {
            json_value_t* obj = json_object(ator, 3);
            bool eok = obj != NULL;
            eok = eok && json_object_set(obj,
                json_const_string(ator, "idx_start"),
                json_number_integer(ator, value->as.srcref.idx_start));
            eok = eok && json_object_set(obj,
                json_const_string(ator, "idx_end"),
                json_number_integer(ator, value->as.srcref.idx_end));
            eok = eok && json_object_set(obj,
                json_const_string(ator, "text"),
                json_string(ator, 
                    srcref_ptr(value->as.srcref),
                    srcref_len(value->as.srcref)));
            if(eok == false)
                return NULL;
            return json_value_wrapper(ator, tag_name, obj);
        } break;
        case AST_ARRAY: {
            json_value_t* array = json_array(ator, value->size > 0 ? value->size : 1);
            for(int i = 0; i < value->size; i++) {
                ast_t* ast_expr = value->as.items[i];
                if(json_array_append(array, ast_to_json(ator, ast_expr)) == false)
                    return NULL;
            }
            return json_value_wrapper(ator, tag_name, array);
        } break;
        default: {

            if(value->size == 1) {
                return json_value_wrapper(ator, tag_name,
                    ast_to_json(ator, value->as.items[0]));
            }

            json_value_t* items = json_array(ator, value->size > 0 ? value->size : 1);
            for(int i = 0; i < value->size; i++) {
                ast_t* ast_expr = value->as.items[i];
                if(json_array_append(items, ast_to_json(ator, ast_expr)) == false)
                    return NULL;
            }
            
            return json_value_wrapper(ator, tag_name, items);

        } break;
    }
}


void ast_print(ast_t* node) {
    arena_t* arena = arena_create(512);
    json_value_t* json = ast_to_json(arena, node);
    char* jstr = json_dumps(json, 2);
    sh_log(jstr);
    free(jstr);
    arena_destroy(arena);
}

