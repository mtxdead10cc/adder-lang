#include "shared/sh_json.h"
#include "shared/sh_parse_tools.h"
#include "shared/sh_arena.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

json_value_t* json_string(arena_t* allocator, char* value, ptrdiff_t len) {
    json_value_t* jval = (json_value_t*) aalloc(allocator, sizeof(json_value_t));
    if( jval == NULL )
        return NULL;
    *jval = (json_value_t) {
        .type = JSON_VALUE_STRING,
        .as.string = (json_string_t) {
            .length = len,
            .text = value
        }
    };
    return jval;
}

json_value_t* json_const_string(arena_t* allocator, const char* value) {
    return json_string(allocator, (char*)value, strlen(value));
}

json_value_t* json_null(arena_t* allocator) {
    json_value_t* jval = (json_value_t*) aalloc(allocator, sizeof(json_value_t));
    if( jval == NULL )
        return NULL;
    *jval = (json_value_t) {
        .type = JSON_VALUE_NULL,
        .as.number_integer = 0
    };
    return jval;
}

json_value_t* json_boolean(arena_t* allocator, bool value) {
    json_value_t* jval = (json_value_t*) aalloc(allocator, sizeof(json_value_t));
    if( jval == NULL )
        return NULL;
    *jval = (json_value_t) {
        .type = JSON_VALUE_BOOLEAN,
        .as.boolean = value
    };
    return jval;
}

json_value_t* json_number_double(arena_t* allocator, double value) {
    json_value_t* jval = (json_value_t*) aalloc(allocator, sizeof(json_value_t));
    if( jval == NULL )
        return NULL;
    *jval = (json_value_t) {
        .type = JSON_VALUE_NUMBER_DOUBLE,
        .as.number_double = value
    };
    return jval;
}

json_value_t* json_number_integer(arena_t* allocator, long value) {
    json_value_t* jval = (json_value_t*) aalloc(allocator, sizeof(json_value_t));
    if( jval == NULL )
        return NULL;
    *jval = (json_value_t) {
        .type = JSON_VALUE_NUMBER_INTEGER,
        .as.number_integer = value
    };
    return jval;
}

json_value_t* json_error(arena_t* allocator, json_tt_t expected, pt_token_t token) {

    json_value_t* jval = (json_value_t*) aalloc(allocator, sizeof(json_value_t));
    if( jval == NULL )
        return NULL;

    *jval = (json_value_t) {
        .type = JSON_VALUE_ERROR,
        .as.error = (json_error_t) {
            .expected = expected,
            .got = (json_string_t) {
                .length = token.length,
                .text = token.text
            }
        }
    };

    return jval;
}

json_value_t* json_array(arena_t* allocator, ptrdiff_t capacity) {

    json_value_t* array = (json_value_t*) aalloc(allocator, sizeof(json_value_t));
    if( array == NULL )
        return NULL;

    json_value_t** content = (json_value_t**) aalloc(allocator, sizeof(json_value_t*) * capacity);
    if( content == NULL )
        return NULL;

    *array = (json_value_t) {
        .type = JSON_VALUE_ARRAY,
        .as.array = (json_array_t) {
            .allocator = allocator,
            .capacity = capacity,
            .size = 0,
            .values = content
        }
    };

    return array;
}

bool json_array_append(json_value_t* json_array, json_value_t* value) {

    if( json_array == NULL )
        return false;

    if( json_array->type != JSON_VALUE_ARRAY )
        return false;

    json_array_t* array = &json_array->as.array;

    if(array->allocator == NULL)
        return false;

    if( array->capacity <= array->size ) {
        
        assert(array->capacity > 0);
        assert(array->size >= 0);

        ptrdiff_t newcap = array->capacity * 2;
        json_value_t** content = (json_value_t**) arealloc(
            array->allocator,
            array->values,
            sizeof(json_value_t*) * newcap);

        if( content == NULL )
            return false;

        array->capacity = newcap;
        array->values = content;
    }

    array->values[array->size] = value;
    array->size += 1;

    return true;
}

json_value_t* json_object(arena_t* allocator, ptrdiff_t capacity) {

    json_value_t* object = (json_value_t*) aalloc(allocator, sizeof(json_value_t));
    if( object == NULL )
        return NULL;

    json_value_t** values = (json_value_t**) aalloc(allocator, sizeof(json_value_t*) * capacity);
    if( values == NULL )
        return NULL;

    json_value_t** keys = (json_value_t**) aalloc(allocator, sizeof(json_value_t*) * capacity);
    if( keys == NULL )
        return NULL;

    *object = (json_value_t) {
        .type = JSON_VALUE_OBJECT,
        .as.object = (json_object_t) {
            .allocator = allocator,
            .capacity = capacity,
            .keys = keys,
            .values = values,
            .size = 0,
        }
    };

    return object;
}

bool json_object_append(json_value_t* json_object, json_value_t* key, json_value_t* value) {

    if( json_object == NULL )
        return false;

    if( json_object->type != JSON_VALUE_OBJECT )
        return false;

    json_object_t* object = &json_object->as.object;

    if( object->allocator == NULL )
        return false;
    
    if( object->capacity <= object->size ) {
        assert(object->capacity > 0);
        assert(object->size >= 0);
        ptrdiff_t newcap = object->capacity * 2;
        json_value_t** values = (json_value_t**) arealloc(
            object->allocator,
            object->values,
            sizeof(json_value_t*) * newcap);
        if( values == NULL )
            return false;
        json_value_t** keys = (json_value_t**) arealloc(
            object->allocator,
            object->keys,
            sizeof(json_value_t*) * newcap);
        if( keys == NULL ) {
            return false;
        }
        object->capacity = newcap;
        object->values = values;
        object->keys = keys;
    }

    object->values[object->size] = value;
    object->keys[object->size] = key;
    object->size += 1;

    return true;
}

bool json_string_value_equals(json_value_t* a, json_value_t* b) {
    if( a->type != JSON_VALUE_STRING )
        return false;
    if( b->type != a->type )
        return false;
    if(a->as.string.length != b->as.string.length)
        return false;
    if(a->as.string.text == b->as.string.text)
        return true;
    ptrdiff_t len = a->as.string.length;
    for(ptrdiff_t i = 0; i < len; i++) {
        if( a->as.string.text[i] != b->as.string.text[i] )
            return false;
    }
    return true;
}

bool json_object_set(json_value_t* json_object, json_value_t* key, json_value_t* value) {

    if(json_object == NULL)
        return false;

    json_object_t* obj = &json_object->as.object;
    for(int i = 0; i < obj->size; i++) {
        if( json_string_value_equals(obj->keys[i], key) ) {
            obj->values[i] = value;
            return true;
        }
    }

    return json_object_append(json_object, key, value);
}

json_value_t* json_object_get(json_value_t* json_object, char* key, ptrdiff_t key_len) {
    if( key == NULL || json_object == NULL )
        return NULL;
    json_object_t* obj = &json_object->as.object;
    for(int i = 0; i < obj->size; i++) {
        if( strncmp(key, obj->keys[i]->as.string.text, key_len) == 0 )
            return obj->values[i];
    }
    return NULL;
}

json_value_t* json_object_get_const(json_value_t* json_object, const char* key) {
    if( key == NULL || json_object == NULL )
        return NULL;
    return json_object_get(json_object, (char*) key, strlen(key));
}

char* json_dumps_append_double(char* prev, double val) {
    char* prevstr = prev != NULL ? prev : ""; 
    char* next = malloc(snprintf(NULL, 0, "%s%lf", prevstr, val) + 1);
    sprintf(next, "%s%lf", prevstr, val);
    if( prev != NULL )
        free(prev);
    return next;
}

char* json_dumps_append_integer(char* prev, long val) {
    char* prevstr = prev != NULL ? prev : ""; 
    char* next = malloc(snprintf(NULL, 0, "%s%ld", prevstr, val) + 1);
    sprintf(next, "%s%ld", prevstr, val);
    if( prev != NULL )
        free(prev);
    return next;
}

char* json_dumps_append_string(char* prev, char* val) {
    char* prevstr = prev != NULL ? prev : ""; 
    char* next = malloc(snprintf(NULL, 0, "%s%s", prevstr, val) + 1);
    sprintf(next, "%s%s", prevstr, val);
    if( prev != NULL )
        free(prev);
    return next;
}

char* json_dumps_append_lenstring(char* prev, char* val, int len) {
    char* prevstr = prev != NULL ? prev : ""; 
    char* next = malloc(snprintf(NULL, 0, "%s%.*s", prevstr, len, val) + 1);
    sprintf(next, "%s%.*s", prevstr, len, val);
    if( prev != NULL )
        free(prev);
    return next;
}

char* json_apply_indent(char* prev, int level, int indent) {
    if( indent <= 0 )
        return prev;
    int len = level * indent;
    if( len < 1 )
        len = 1;
    char tmp[len+1];
    memset(tmp, ' ', len+1);
    tmp[0] = '\n';
    tmp[len] = '\0';
    return json_dumps_append_string(prev, tmp);
}

char* json_internal_dumps(json_value_t* json, int level, int indent) {
    switch(json->type) {
        case JSON_VALUE_NULL:
            return json_dumps_append_string(NULL, "null");
        case JSON_VALUE_NUMBER_DOUBLE:
            return json_dumps_append_double(NULL, json->as.number_double);
        case JSON_VALUE_NUMBER_INTEGER:
            return json_dumps_append_integer(NULL, json->as.number_integer);
        case JSON_VALUE_STRING: {
            json_string_t jstr = json->as.string;
            char* str = json_dumps_append_string(NULL, "\"");
            str = json_dumps_append_lenstring(str, jstr.text, jstr.length);
            str = json_dumps_append_string(str, "\"");
            return str;
        } break;
        case JSON_VALUE_BOOLEAN: {
            if(json->as.boolean)
                return json_dumps_append_string(NULL, "true");
            else
                return json_dumps_append_string(NULL, "false");
        } break;
        case JSON_VALUE_ARRAY: {
            char* str = json_dumps_append_string(NULL, "[");
            ptrdiff_t size = json->as.array.size;
            for(int i = 0; i < size; i++) {
                str = json_apply_indent(str, level + 1, indent);
                char* inner = json_internal_dumps(
                    json->as.array.values[i],
                    level + 2,
                    indent);
                str = json_dumps_append_string(str, inner);
                free(inner);
                if( i < (size - 1) )
                    str = json_dumps_append_string(str, ",");
            }
            if( size > 0 )
                str = json_apply_indent(str, level-1, indent);
            str = json_dumps_append_string(str, "]");
            return str;
        } break;
        case JSON_VALUE_OBJECT: {
            char* str = json_dumps_append_string(NULL, "{");
            ptrdiff_t size = json->as.object.size;
            for(int i = 0; i < size; i++) {
                str = json_apply_indent(str, level + 1, indent);
                char* keystr = json_internal_dumps(
                    json->as.object.keys[i], 0, 0);
                str = json_dumps_append_string(str, keystr);
                free(keystr);
                str = json_dumps_append_string(str, ":");
                if( indent > 0 )
                    str = json_dumps_append_string(str, " ");
                char* inner = json_internal_dumps(
                    json->as.object.values[i],
                    level + 2,
                    indent);
                str = json_dumps_append_string(str, inner);
                free(inner);
                if( i < (size - 1) )
                    str = json_dumps_append_string(str, ",");
            }
            if( size > 0 )
                str = json_apply_indent(str, level-1, indent);
            str = json_dumps_append_string(str, "}");
            return str;
        } break;
        case JSON_VALUE_ERROR: {
            char* str = json_dumps_append_string(NULL, "<error: expected '");
            str = json_dumps_append_string(str, json_tt_to_string(json->as.error.expected));
            str = json_dumps_append_string(str, "' but got ");
            if( json->as.error.got.text != NULL ) {
                str = json_dumps_append_string(str, "'");
                str = json_dumps_append_lenstring(str,
                    json->as.error.got.text,
                    (int)json->as.error.got.length);
                str = json_dumps_append_string(str, "'");
            } else {
                str = json_dumps_append_string(str, "null-pointer");
            }
            str = json_dumps_append_string(str, ">");
            return str;
        } break;
        default: {
            return json_dumps_append_string(NULL, "<UNK>");
        } break;
    }
}

char* json_dumps(json_value_t* json, int indent) {
    return json_internal_dumps(json, 0, indent);
}


char* json_tt_to_string(json_tt_t tt) {
    switch(tt) {
        case JTT_NULL:              return "JTT_NULL";
        case JTT_NUMBER_INTEGER:    return "JTT_NUMBER_INTEGER";
        case JTT_NUMBER_DOUBLE:     return "JTT_NUMBER_DOUBLE";
        case JTT_STRING:            return "JTT_STRING";
        case JTT_BOOL:              return "JTT_BOOL";
        case JTT_SEPARATOR:         return "JTT_SEPARATOR";
        case JTT_ARRAY_OPEN:        return "JTT_ARRAY_OPEN";
        case JTT_ARRAY_CLOSE:       return "JTT_ARRAY_CLOSE";
        case JTT_OBJECT_OPEN:       return "JTT_OBJECT_OPEN";
        case JTT_OBJECT_CLOSE:      return "JTT_OBJECT_CLOSE";
        case JTT_KVP_SEPARATOR:     return "JTT_KVP_SEPARATOR";
        default:                    return "<UNK JTT>";
    }
}

ptrdiff_t json_scan(char* text, ptrdiff_t remaining, int64_t* type_out) {

    *type_out = -1;

    if(remaining <= 0)
        return 0;

    switch(text[0]) {
        case ',': {
            *type_out = JTT_SEPARATOR;
        } break;
        case ':': {
            *type_out = JTT_KVP_SEPARATOR;
        } break;
        case '[': {
            *type_out = JTT_ARRAY_OPEN;
        } break;
        case ']': {
            *type_out = JTT_ARRAY_CLOSE;
        } break;
        case '{': {
            *type_out = JTT_OBJECT_OPEN;
        } break;
        case '}': {
            *type_out = JTT_OBJECT_CLOSE;
        } break;
        default: break;
    }

    if( *type_out >= 0 )
        return 1;

    if(pt_str_eq(text, remaining, "false")) {
        *type_out = JTT_BOOL;
        return pt_str_const_len("false");
    }

    if(pt_str_eq(text, remaining, "true")) {
        *type_out = JTT_BOOL;
        return pt_str_const_len("true");
    }

    if(pt_str_eq(text, remaining, "null")) {
        *type_out = JTT_NULL;
        return pt_str_const_len("null");
    }

    ptrdiff_t consumed = 0;

    consumed = pt_scan_number(text, remaining);
    if( consumed > 0 ) {
        *type_out = JTT_NUMBER_INTEGER;
        if( pt_string_contains_char(text, consumed, '.') )
            *type_out = JTT_NUMBER_DOUBLE;
        return consumed;
    }

    consumed = pt_scan_string(text, remaining);
    if( consumed > 0 ) {
        *type_out = JTT_STRING;
        return consumed;
    }
    
    return 0;
}

json_value_t* json_parse_value(arena_t* allocator, pt_state_t* state) {

    pt_token_t tok = pt_state_peek(state, 0);

    switch(tok.type) {
        case JTT_BOOL: {
            json_value_t* result = NULL;
            if( pt_str_eq(tok.text, tok.length, "true") )
                result = json_boolean(allocator, true);
            else
                result = json_boolean(allocator, false);
            pt_state_advance(state);
            return result;
        } break;
        case JTT_NUMBER_DOUBLE: {
            json_value_t* result = json_number_double(allocator, strtod(tok.text, NULL));
            pt_state_advance(state);
            return result;
        } break;
        case JTT_NUMBER_INTEGER: {
            json_value_t* result = json_number_integer(allocator, (long) strtod(tok.text, NULL));
            pt_state_advance(state);
            return result;
        } break;
        case JTT_STRING: {
            assert(tok.length >= 2);
            json_value_t* result = json_string(allocator, tok.text+1, tok.length-2);
            pt_state_advance(state);
            return result;
        } break;
        case JTT_NULL: {
            pt_state_advance(state);
            return json_null(allocator);
        } break;
        case JTT_ARRAY_OPEN: {

            json_value_t* result = json_array(allocator, 8);

            pt_state_advance(state);

            while( pt_state_has_tokens(state) ) {

                if(pt_state_advance_if(state, JTT_ARRAY_CLOSE))
                    break;

                json_value_t* inner = json_parse_value(allocator, state);
                json_array_append(result, inner);
                pt_state_advance_if(state, JTT_SEPARATOR);
            }

            return result;
        } break;
        case JTT_OBJECT_OPEN: {

            json_value_t* result = json_object(allocator, 8);

            pt_state_advance(state);

            while( pt_state_has_tokens(state) ) {

                if( pt_state_advance_if(state, JTT_OBJECT_CLOSE) )
                    break;

                json_value_t* key = NULL;
                if( pt_state_match_any(state, JTT_STRING) )
                    key = json_parse_value(allocator, state);
                else {
                    key = json_error(allocator, JTT_STRING, pt_state_peek(state, 0));
                    pt_state_advance(state);
                }

                pt_state_advance_if(state, JTT_KVP_SEPARATOR);

                json_value_t* value = json_parse_value(allocator, state);
                json_object_append(result, key, value);

                pt_state_advance_if(state, JTT_SEPARATOR);
            }

            return result;
        } break;
    }

    
    json_value_t* err = json_error(allocator, JTT_UNKNOWN, tok);
    pt_state_advance(state);
    return err;
}

json_value_t* json_parse(arena_t* allocator, char* json_str, ptrdiff_t len) {
    
    pt_state_t p = (pt_state_t){ 0 };

    pt_result_code_t res = pt_state_init(&p, json_scan, json_str, len);

    if( res > 0 )
        return json_null(allocator); // error | nothing parsed

    json_value_t* result = json_parse_value(allocator, &p);
    
    pt_state_free(&p);

    return result;
}