#include "adrcom/ast/co_ast_json.h"
#include <shared/sh_json.h>

typedef struct srcpath_t {
    char*       path;
    size_t      length;
} srcpath_t;

typedef struct srcset_t {
    size_t      capacity;
    size_t      size;
    srcpath_t*  paths;
} srcset_t;

bool srcset_init(srcset_t* set, int capacity) {
    srcpath_t* paths = (srcpath_t*) malloc(capacity * sizeof(srcset_t));
    if(paths == NULL)
        return false;
    set->capacity = capacity;
    set->size = 0;
    set->paths = paths;
    return true;
}

void srcset_destroy(srcset_t* set) {
    if(set->paths != NULL)
        free(set->paths);
    *set = (srcset_t) {0};
}

int srcset_find_index(srcset_t* set, srcpath_t path) {
    for(size_t i = 0; i < set->size; i++) {
        if(set->paths[i].length != path.length)
            continue;
        if(strncmp(set->paths[i].path, path.path, path.length) == 0)
            return i;
    }
    return -1;
}

int srcset_add(srcset_t* set, srcpath_t path) {
    
    int index = srcset_find_index(set, path);
    if( index >= 0 )
        return index;

    if( set->size >= set->capacity ) {
        int capacity = set->size * 2;
        srcpath_t* paths = (srcpath_t*) realloc(set->paths,
            capacity * sizeof(srcpath_t));
        if(paths == NULL)
            return -1;
        set->capacity = capacity;
        set->paths = paths;
    }

    index = set->size;
    set->paths[index] = path;
    set->size += 1;
    return index;
}

json_value_t* json_value_wrapper(arena_t* ator, const char* tag_name, json_value_t* value) {

    if(ator == NULL || value == NULL)
        return NULL;

    assert(strnlen(tag_name, 5) >= 4); // "AST_", removing the 4 initial chars

    json_value_t* wrapper = json_object(ator, 2);
    json_value_t* wrptagkey = json_const_string(ator, "tag");
    json_value_t* wrptagname = json_const_string(ator, tag_name + 4);
    json_value_t* valuekey = json_const_string(ator,
        (value->type == JSON_VALUE_ARRAY)
            ? "items"
            : "value");

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

json_value_t* srcref_to_json(arena_t* ator, ast_t* value, srcset_t* set) {

    srcpath_t path = (srcpath_t) {
        .path = value->as.srcref.src->path,
        .length = value->as.srcref.src->path_length
    };

    int index = srcset_add(set, path);

    json_value_t* obj = json_object(ator, 4);

    bool eok = index >= 0 && obj != NULL;

    eok = eok && json_object_set(obj,
        json_const_string(ator, "idx_start"),
        json_number(ator, value->as.srcref.idx_start));

    eok = eok && json_object_set(obj,
        json_const_string(ator, "idx_end"),
        json_number(ator, value->as.srcref.idx_end));

    eok = eok && json_object_set(obj,
        json_const_string(ator, "source"),
        json_number(ator, index));

    eok = eok && json_object_set(obj,
        json_const_string(ator, "text"),
        json_string(ator, 
            srcref_ptr(value->as.srcref),
            srcref_len(value->as.srcref)));

    return eok ? obj : NULL;
}

json_value_t* ast_to_json_internal(arena_t* ator, ast_t* value, srcset_t* set) {

    if(ator == NULL || value == NULL)
        return NULL;

    const char* tag_name = ast_tag_to_string(value->tag);

    switch(value->tag) {
        case AST_INT:        return json_value_wrapper(ator,tag_name,
            json_number(ator, value->as.value_int));
        case AST_BOOL:       return json_value_wrapper(ator, tag_name,
            json_boolean(ator, value->as.value_bool));
        case AST_CHAR:       return json_value_wrapper(ator, tag_name,
            json_string(ator, &value->as.value_char, 1));
        case AST_FLOAT:      return json_value_wrapper(ator, tag_name,
            json_number(ator, value->as.value_float));
        case AST_UNDEFINED:  return json_value_wrapper(ator, tag_name,
            json_null(ator));
        case AST_STRING: // Fallthrough
        case AST_SYMBOL: {
            return json_value_wrapper(ator, tag_name,
                srcref_to_json(ator, value, set));
        } break;
        default: {
            json_value_t* items = json_array(ator, value->size > 0 ? value->size : 1);
            for(int i = 0; i < value->size; i++) {
                ast_t* ast_expr = value->as.items[i];
                if(json_array_append(items, ast_to_json_internal(ator, ast_expr, set)) == false)
                    return NULL;
            }
            return json_value_wrapper(ator, tag_name, items);
        } break;
    }
}

json_value_t* ast_to_json(arena_t* ator, ast_t* value) {

    if(ator == NULL || value == NULL)
        return NULL;
    
    srcset_t set = { 0 };

    if( srcset_init(&set, 4) == false )
        return NULL;

    json_value_t* ast = ast_to_json_internal(ator, value, &set);
    json_value_t* sources = json_array(ator, set.size);

    for(size_t i = 0; i < set.size; i++) {
        json_array_append(sources,
            json_string(ator,
                set.paths[i].path,
                set.paths[i].length));
    }

    srcset_destroy(&set);

    json_value_t* result = json_object(ator, 2);

    json_object_set(result,
        json_const_string(ator, "sources"),
        sources);

    json_object_set(result,
        json_const_string(ator, "ast"),
        ast);

    return result;
}

typedef struct jtlut_entry_t {
    char*       str;
    ast_tag_t   tag;
} jtlut_entry_t;

int jtlut_cmp(const void* a, const void* b) {
    return strcmp(
        ((jtlut_entry_t*) a)->str,
        ((jtlut_entry_t*) b)->str);
}

void jtlut_init(jtlut_entry_t* table, int count) {
    for(int i = 0; i < count; i++) {
        table[i].str = (char*) ast_tag_to_string((ast_tag_t)i) + 4;
        table[i].tag = (ast_tag_t) i;
    }
    qsort(table,
        count,
        sizeof(jtlut_entry_t),
        jtlut_cmp);
}

size_t strcnt(char* string, char stopchar) {
    size_t maxlen = strlen(string);
    for(size_t i = 0; i < maxlen; i++) {
        if(string[i] == stopchar)
            return i;
    }
    return maxlen;
}

ast_tag_t jtlut_binsearch(jtlut_entry_t* table, char* str, int sta, int end) {

    size_t str_len = strcnt(str, '"');

    int dif = end - sta;
    if( dif <= 6 ) {
        for(int i = sta; i <= end; i++) {
            if(strncmp(str, table[i].str, str_len) == 0)
                return table[i].tag;
        }
        return AST_UNDEFINED;
    }

    int mid = sta + (dif / 2);
    int cmp = strncmp(str, table[mid].str, str_len);
    if(cmp < 0)
        return jtlut_binsearch(table, str, sta, mid);
    else if(cmp > 0)
        return jtlut_binsearch(table, str, mid, end);
    else
        return table[mid].tag;

}

typedef struct ast_json_params_t {
    arena_t*        arena;
    jtlut_entry_t*  table;
    src_t**         srcs;
    size_t          srccount;
} ast_json_params_t;

ast_tag_t ast_tag_from_json(ast_json_params_t* params, json_value_t* json) {
    if(json_is_object(json) == false)
        return AST_UNDEFINED;
    json_value_t* jtag = json_object_get(json, "tag");
    if(json_is_string(jtag) == false)
        return AST_UNDEFINED;
    return jtlut_binsearch(params->table, jtag->as.string.text, 0, AST__COUNT-1);
}

srcref_t srcref_from_json(ast_json_params_t* params, json_value_t* json) {

    if(json_is_object(json) == false)
        return (srcref_t){0};

    json_value_t* source_index = json_object_get(json, "source");
    if(json_is_number(source_index) == false)
        return (srcref_t){0};

    json_value_t* start_idx = json_object_get(json, "idx_start");
    if(json_is_number(start_idx) == false)
        return (srcref_t){0};

    json_value_t* end_idx = json_object_get(json, "idx_end");
    if(json_is_number(end_idx) == false)
        return (srcref_t){0};

    size_t srcptr_index = source_index->as.number;
    if(srcptr_index >= params->srccount)
        return (srcref_t){0};

    return (srcref_t) {
        .idx_end = (size_t) end_idx->as.number,
        .idx_start = (size_t) start_idx->as.number,
        .src = params->srcs[srcptr_index]
    };
}

ast_t* ast_value_int(ast_json_params_t* params, json_value_t* json) {
    json_value_t* value = json_object_get(json, "value");
    if(json_is_number(value) == false)
        return NULL;
    return ast_int(params->arena, (value->as.number + 0.5));
}

ast_t* ast_value_float(ast_json_params_t* params, json_value_t* json) {
    json_value_t* value = json_object_get(json, "value");
    if(json_is_number(value) == false)
        return NULL;
    return ast_float(params->arena, value->as.number);
}

ast_t* ast_value_string(ast_json_params_t* params, json_value_t* json) {
    json_value_t* value = json_object_get(json, "value");
    if(json_is_object(value) == false)
        return NULL;
    srcref_t ref = srcref_from_json(params, value);
    if(srcref_is_valid(ref) == false)
        return NULL;
    return ast_string(params->arena, ref);
}

ast_t* ast_value_symbol(ast_json_params_t* params, json_value_t* json) {
    json_value_t* value = json_object_get(json, "value");
    if(json_is_object(value) == false)
        return NULL;
    srcref_t ref = srcref_from_json(params, value);
    if(srcref_is_valid(ref) == false)
        return NULL;
    return ast_symbol(params->arena, ref);
}

ast_t* ast_value_bool(ast_json_params_t* params, json_value_t* json) {
    json_value_t* value = json_object_get(json, "value");
    if(json_is_bool(value) == false)
        return NULL;
    return ast_bool(params->arena, value->as.boolean);
}

ast_t* ast_value_char(ast_json_params_t* params, json_value_t* json) {
    json_value_t* value = json_object_get(json, "value");
    if(json_is_string(value) == false)
        return NULL;
    if(value->as.string.length < 1)
        return NULL;
    return ast_char(params->arena, value->as.string.text[0]);
}

ast_t* ast_from_json_internal(ast_json_params_t* params, json_value_t* json) {

    // todo: proper validation of fields
    // todo: common error structure

    ast_tag_t tag = ast_tag_from_json(params, json);

    switch(tag) {
        case AST_UNDEFINED: return NULL;
        case AST_FLOAT:     return ast_value_float(params, json);
        case AST_INT:       return ast_value_int(params, json);
        case AST_BOOL:      return ast_value_bool(params, json);
        case AST_CHAR:      return ast_value_char(params, json);
        case AST_STRING:    return ast_value_string(params, json);
        case AST_SYMBOL:    return ast_value_symbol(params, json);
        case AST_ARRAY: {
            json_value_t* value = json_object_get(json, "items");
            if(json_is_array(value) == false)
                return NULL;
            ast_t* arr = ast_array(params->arena); // error
            ptrdiff_t size = json_get_size(value);
            for(ptrdiff_t i = 0; i < size; i++) {
                json_value_t* arrval = json_array_get(value, i);
                ast_t* arrastval = ast_from_json_internal(params, arrval);
                assert(arrastval != NULL); // error
                ast_array_append(params->arena, arr, arrastval); // error
            }
            return arr;
        }
        default: {
            json_value_t* jitems = json_object_get(json, "items");
            if(jitems == NULL)
                return NULL;
            int size = json_get_size(jitems);
            // TODO: validate json size == expected ast size
            ast_t* node = ast(params->arena, tag, size);
            for(int i = 0; i < size; i++) {
                json_value_t* item = json_array_get(jitems, i);
                ast_t* astitem = ast_from_json_internal(params, item);
                assert(astitem != NULL); // error
                node->as.items[i] = astitem;
            }
            return node; 
        }
    }   
}

bool srcset_from_json(srcset_t* set, json_value_t* sources) {
    
    if(json_is_array(sources) == false)
        return false;
    
    ptrdiff_t len = json_get_size(sources);
    if(srcset_init(set, len) == false)
        return false;
    
    for(ptrdiff_t i = 0; i < len; i++) {
        json_value_t* value = json_array_get(sources, i);
        if(json_is_string(value) == false) {
            sh_log_error("srcset_from_json: invalid source path in json array");
            continue;
        }
        srcpath_t path = {
            .path = value->as.string.text,
            .length = value->as.string.length
        };
        if( srcset_add(set, path) < 0 )
            sh_log_error("srcset_from_json: internal error");
    }

    return (size_t) len == set->size;
}

ast_t* ast_from_json(arena_t* ator, json_value_t* json, mksrc_fn_t srcmaker, void* user) {

    jtlut_entry_t table[AST__COUNT] = {0};
    jtlut_init(table, AST__COUNT);

    json_value_t* json_sources = json_object_get(json, "sources");
    if( json_is_array(json_sources) == false )
        return NULL;

    srcset_t set = {0};
    if(srcset_from_json(&set, json_sources) == false)
        return NULL;

    src_t* sources[set.size];
    for(size_t i = 0; i < set.size; i++) {
        sources[i] = srcmaker(user,
            set.paths[i].path,
            set.paths[i].length);
    }

    ast_json_params_t params = {
        .arena = ator,
        .table = table,
        .srccount = set.size,
        .srcs = sources
    };

    srcset_destroy(&set);

    json_value_t* json_ast = json_object_get(json, "ast");
    if( json_is_object(json_ast) == false )
        return NULL;
    
    return ast_from_json_internal(&params, json_ast);
}

void ast_print_json(ast_t* node) {
    arena_t* arena = arena_create(512);
    json_value_t* json = ast_to_json(arena, node);
    char* jstr = json_dumps(json, 1);
    sh_log(jstr);
    free(jstr);
    arena_destroy(arena);
}