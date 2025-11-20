#include "adrcom/ast/co_ast_expr.h"
#include "shared/sh_json.h"

ast_kvp_value_t ast_value_int(int value) {
    return (ast_kvp_value_t) {
        .tag = AVAL_INT,
        .u.value_int = value
    };
}

ast_kvp_value_t ast_value_float(float value) {
    return (ast_kvp_value_t) {
        .tag = AVAL_FLOAT,
        .u.value_float = value
    };
}

ast_kvp_value_t ast_value_bool(bool value) {
    return (ast_kvp_value_t) {
        .tag = AVAL_BOOL,
        .u.value_bool = value
    };
}

ast_kvp_value_t ast_value_char(char value) {
    return (ast_kvp_value_t) {
        .tag = AVAL_CHAR,
        .u.value_char = value
    };
}

ast_kvp_value_t ast_value_string(srcref_t value) {
    return (ast_kvp_value_t) {
        .tag = AVAL_SRCREF,
        .u.srcref = value
    };
}

ast_kvp_value_t ast_value_srcref(srcref_t value) {
    return (ast_kvp_value_t) {
        .tag = AVAL_SRCREF,
        .u.srcref = value
    };
}

ast_kvp_value_t ast_value_expr(ast_expr_t* ptr) {
    return (ast_kvp_value_t) {
        .tag = AVAL_EXPR,
        .u.expr = ptr
    };
}

ast_kvp_value_t ast_value_list(arena_t* allocator, int capacity) {
    ast_kvp_value_t list = (ast_kvp_value_t) { 0 };
    list.u.list.content = aalloc(allocator, capacity * sizeof(ast_kvp_value_t));
    if( list.u.list.content == NULL ) {
        assert( false );
        return (ast_kvp_value_t) { 0 };
    }
    list.tag = AVAL_LIST;
    list.u.list.capacity = capacity;
    list.u.list.size = 0;
    return list;
}

bool ast_value_list_append(arena_t* allocator, ast_kvp_value_t* list_value, ast_kvp_value_t value) {

    if( list_value == NULL )
        return false;

    if( list_value->tag != AVAL_LIST )
        return false;

    ast_list_t* list = &list_value->u.list;

    if( list->capacity <= list->size ) {
        assert(list->capacity > 0);
        assert(list->size >= 0);
        ptrdiff_t newcap = list->capacity * 2;
        ast_kvp_value_t* content = arealloc(allocator,
            list->content,
            sizeof(ast_kvp_value_t) * newcap);
        if( content == NULL )
            return false;
        list->capacity = newcap;
        list->content = content;
    }

    list->content[list->size] = value;
    list->size += 1;

    return true;
}

ast_kvp_value_t ast_value_list_get(ast_kvp_value_t* list_value, int index) {
    if( list_value == NULL )
        return (ast_kvp_value_t) { 0 };
    ast_list_t* list = &list_value->u.list;
    if( index < 0 || list->size <= index )
        return (ast_kvp_value_t) { 0 };
    return list->content[index];
}

ast_expr_t ast_expr(ast_expr_tag_t tag) {
    return (ast_expr_t) {
        .tag = tag,
        .map = {{ 0 }}
    };
}

void ast_expr_set(ast_expr_t* node, ast_key_t key, ast_kvp_value_t value) {
    assert(node != NULL);
    assert(key >= 0 && key < AKEY_COUNT);
    node->map[key] = value;
}

ast_kvp_value_t ast_expr_get(ast_expr_t* node, ast_key_t key) {
    assert(node != NULL);
    assert(key >= 0 && key < AKEY_COUNT);
    return node->map[key];
}

const char* ast_kvp_value_tag_to_string(ast_kvp_value_tag_t tag) {
    switch (tag) {
        case AVAL_UNDEFINED:    return "undefined";
        case AVAL_EXPR:         return "expression";
        case AVAL_LIST:         return "list";
        case AVAL_SRCREF:       return "srcref";
        case AVAL_INT:          return "int";
        case AVAL_FLOAT:        return "float";
        case AVAL_BOOL:         return "bool";
        case AVAL_CHAR:         return "char";
        default:                return "unknown (invalid value tag)";
    }
}

const char* ast_key_to_string(ast_key_t key) {
    switch (key) {
        case AKEY_NAME:             return "name";
        case AKEY_SOURCE:           return "source";
        case AKEY_VALUE:            return "value";
        case AKEY_RETURN:           return "return";
        case AKEY_ARGS:             return "args";
        case AKEY_BODY:             return "body";
        case AKEY_IF_CONDITION:     return "if_condition";
        case AKEY_IF_TRUE:          return "if_true";
        case AKEY_IF_NEXT:          return "if_next";
        case AKEY_LOOP_VAR_DECL:    return "loop_var_decl";
        case AKEY_LOOP_COLLECTION:  return "loop_collection";
        case AKEY_LOOP_BODY:        return "loop_body";
        case AKEY_LEFT:             return "left";
        case AKEY_RIGHT:            return "right";
        case AKEY_INNER:            return "inner";
        case AKEY_COUNT:            return "count (not a valid key)";
        default:                    return "unknown (not a valid key)";
    }
}

const char* ast_expr_tag_to_string(ast_expr_tag_t tag) {
    switch(tag) {
        case ATAG_VALUE_NONE:       return "VALUE_NONE";
        case ATAG_VALUE_INT:        return "VALUE_INT";
        case ATAG_VALUE_BOOL:       return "VALUE_BOOL";
        case ATAG_VALUE_CHAR:       return "VALUE_CHAR";
        case ATAG_VALUE_FLOAT:      return "VALUE_FLOAT";
        case ATAG_UNOP_NOT:         return "UNOP_NOT";
        case ATAG_UNOP_NEG:         return "UNOP_NEG";
        case ATAG_BIN_MUL:          return "BIN_MUL";
        case ATAG_BIN_DIV:          return "BIN_DIV";
        case ATAG_BIN_MOD:          return "BIN_MOD";
        case ATAG_BIN_ADD:          return "BIN_ADD";
        case ATAG_BIN_SUB:          return "BIN_SUB";
        case ATAG_BIN_XOR:          return "BIN_XOR";
        case ATAG_BIN_LT:           return "BIN_LT";
        case ATAG_BIN_GT:           return "BIN_GT";
        case ATAG_BIN_LT_EQ:        return "BIN_LT_EQ";
        case ATAG_BIN_GT_EQ:        return "BIN_GT_EQ";
        case ATAG_BIN_EQ:           return "BIN_EQ";
        case ATAG_BIN_NEQ:          return "BIN_NEQ";
        case ATAG_BIN_OR:           return "BIN_OR";
        case ATAG_BIN_AND:          return "BIN_AND";
        case ATAG_ARRAY:            return "ARRAY";
        case ATAG_IF_CHAIN:         return "IF_CHAIN";
        case ATAG_FOREACH:          return "FOREACH";
        case ATAG_ASSIGN:           return "ASSIGN";
        case ATAG_TYANNOT:          return "TYANNOT";
        case ATAG_VAR_REF:          return "VAR_REF";
        case ATAG_FUN_EXDECL:       return "FUN_EXDECL";
        case ATAG_FUN_DECL:         return "FUN_DECL";
        case ATAG_FUN_CALL:         return "FUN_CALL";
        case ATAG_ARGLIST:          return "ARGLIST";
        case ATAG_RETURN:           return "RETURN";
        case ATAG_BLOCK:            return "BLOCK";
        case ATAG_TYPE_VOID:        return "TYPE_VOID";
        case ATAG_TYPE_FLOAT:       return "TYPE_FLOAT";
        case ATAG_TYPE_INT:         return "TYPE_INT";
        case ATAG_TYPE_CHAR:        return "TYPE_CHAR";
        case ATAG_TYPE_BOOL:        return "TYPE_BOOL";
        case ATAG_TYPE_SOMETIMES:   return "TYPE_SOMETIMES";
        case ATAG_TYPE_ALWAYS:      return "TYPE_ALWAYS";
        case ATAG_TYPE_LIST:        return "TYPE_LIST";
        case ATAG_TYPE_FUNC:        return "TYPE_FUNC";
        case ATAG_TYPE_RETURN:      return "TYPE_RETURN";
        default:                    return "<UNKNOWN>";
    }
}

json_value_t* ast_kvp_value_to_json(ast_kvp_value_t value) {

    const char* tag_name = ast_kvp_value_tag_to_string(value.tag);

    json_value_t* wrapper = json_object(4);
    if( wrapper == NULL ) {
        sh_log_error("ast_kvp_value_to_json: allocation failed");
        return NULL;
    }

    bool export_ok = json_object_set(wrapper,
        json_const_string("tag"),
        json_const_string(tag_name),
        true);

    if( export_ok == false ) {
        sh_log_error("ast_kvp_value_to_json: allocation failed");
        json_free(wrapper);
        return false;
    }

    switch(value.tag) {
        case AVAL_INT: {
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                json_number_integer(value.u.value_int),
                true);
        } break;
        case AVAL_BOOL: {
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                json_boolean(value.u.value_bool),
                true);
        } break;
        case AVAL_CHAR: {
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                json_string(&value.u.value_char, 1),
                true);
        } break;
        case AVAL_FLOAT:{
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                json_number_double(value.u.value_float),
                true);
        } break;
        case AVAL_UNDEFINED:{
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                json_null(),
                true);
        } break;
        case AVAL_EXPR: {
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                ast_expr_to_json(value.u.expr),
                true);
        } break;
        case AVAL_SRCREF: {

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
                json_number_integer(value.u.srcref.idx_start),
                true);

            export_ok = json_object_set(obj,
                json_const_string("idx_end"),
                json_number_integer(value.u.srcref.idx_end),
                true);

            export_ok = json_object_set(obj,
                json_const_string("source"),
                json_string(value.u.srcref.source,
                    strlen(value.u.srcref.source)),
                true);
            
        } break;
        case AVAL_LIST: {

            json_value_t* array = json_array(value.u.list.size);
            export_ok = json_object_set(wrapper,
                json_const_string("value"),
                array,
                true);

            if( export_ok == false ) {
                json_free(array);
                array = NULL;
            }

            for(int i = 0; i < value.u.list.size; i++) {
                ast_kvp_value_t ast_kval = value.u.list.content[i];
                export_ok = export_ok && json_array_append(array,
                    ast_kvp_value_to_json(ast_kval), true);
            }

        } break;
    }

    if( export_ok == false ) {
        sh_log_error("ast_kvp_value_to_json: value allocation failed");
        json_free(wrapper);
        return NULL;
    }

    return wrapper;
}

json_value_t* ast_expr_to_json(ast_expr_t* node) {

    assert(node != NULL);

    if( node == NULL ) {
        sh_log_error("ast_expr_to_json: ast node was null");
        return NULL;
    }

    json_value_t* jnode = json_object(AKEY_COUNT);
    if( node == NULL ) {
        sh_log_error("ast_expr_to_json: out of memory");
        return NULL;
    }

    const char* tag = ast_expr_tag_to_string(node->tag);
    bool export_ok = json_object_set(jnode,
            json_const_string("tag"),
            json_const_string(tag),
            true);

    if( export_ok == false ) {
        sh_log_error("ast_expr_to_json: failed to set tag");
        return NULL;
    }

    json_value_t* jnode_map = json_object(AKEY_COUNT);
    if( jnode_map == NULL ) {
        json_free(jnode);
        sh_log_error("ast_expr_to_json (map): out of memory");
        return NULL;
    }

    export_ok = json_object_set(jnode,
            json_const_string("map"),
            jnode_map,
            true);

    if( export_ok == false ) {
        json_free(jnode);
        json_free(jnode_map);
        sh_log_error("ast_expr_to_json: failed to set map");
        return NULL;
    }

    for(int i = 0; i < AKEY_COUNT; i++) {
        ast_kvp_value_t value = node->map[i];
        if( value.tag == AVAL_UNDEFINED )
            continue;
        export_ok = json_object_set(jnode_map,
            json_const_string(ast_key_to_string(i)),
            ast_kvp_value_to_json(value),
            true);
        if( export_ok == false ) {
            json_free(jnode);
            sh_log_error("ast_expr_to_json: failed to set"
                         " (\"%s\": <value>) pair for node"
                         " with tag %s",
                         ast_key_to_string(i),
                         ast_expr_tag_to_string(node->tag));
            return NULL;
        }
    }

    return jnode;
}

