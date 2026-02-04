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
    if(value == NULL || len < 0)
        return NULL;
    assert(len >= 0);
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
    if(value == NULL)
        return NULL;
    return json_string(allocator, (char*)value, strlen(value));
}

json_value_t* json_null(arena_t* allocator) {
    json_value_t* jval = (json_value_t*) aalloc(allocator, sizeof(json_value_t));
    if( jval == NULL )
        return NULL;
    *jval = (json_value_t) {
        .type = JSON_VALUE_NULL,
        .as.number = 0.0
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

json_value_t* json_number(arena_t* allocator, double value) {
    json_value_t* jval = (json_value_t*) aalloc(allocator, sizeof(json_value_t));
    if( jval == NULL )
        return NULL;
    *jval = (json_value_t) {
        .type = JSON_VALUE_NUMBER,
        .as.number = value
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
            .got = {
                .length = token.length,
                .buffer = token.buffer,
                .start = token.start_offset
            }
        }
    };

    return jval;
}

json_value_t* json_array(arena_t* allocator, ptrdiff_t capacity) {

    json_value_t* array = (json_value_t*) aalloc(allocator, sizeof(json_value_t));
    if( array == NULL )
        return NULL;

    if(capacity <= 0)
        capacity = 1;

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

    if( json_array == NULL || value == NULL )
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

json_value_t* json_array_get(json_value_t* array, ptrdiff_t index) {
    if(json_is_array(array) == false)
        return NULL;
    if(index >= array->as.array.size)
        return NULL;
    return array->as.array.values[index];
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

    if( json_object == NULL || key == NULL || value == NULL )
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
    if( (a == NULL) ^ (b == NULL) )
        return false;
    if( a == b )
        return true;
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

    if( json_object == NULL || key == NULL || value == NULL )
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

json_value_t* json_object_getn(json_value_t* json_object, char* key, ptrdiff_t key_len) {
    if( key == NULL || json_object == NULL )
        return NULL;
    json_object_t* obj = &json_object->as.object;
    for(int i = 0; i < obj->size; i++) {
        if( strncmp(key, obj->keys[i]->as.string.text, key_len) == 0 )
            return obj->values[i];
    }
    return NULL;
}

json_value_t* json_object_get(json_value_t* json_object, const char* key) {
    if( key == NULL || json_object == NULL )
        return NULL;
    return json_object_getn(json_object, (char*) key, strlen(key));
}

bool json_object_has(json_value_t* json_object, const char* key, json_value_type_t value_type) {
    if( key == NULL || json_object == NULL )
        return false;
    json_value_t* value = json_object_getn(json_object,
        (char*) key, strlen(key));
    if( value == NULL )
        return false;
    return value->type == value_type;
}

ptrdiff_t json_get_size(json_value_t* json) {
    switch(json->type) {
        case JSON_VALUE_ARRAY:
            return json->as.array.size;
        case JSON_VALUE_OBJECT:
            return json->as.object.size;
        case JSON_VALUE_BOOLEAN:
        case JSON_VALUE_NUMBER:
        case JSON_VALUE_STRING:
            return 1;
        default:
            return 0;
    }
}

bool json_is_bool(json_value_t* json) {
    if(json == NULL)
        return false;
    return json->type == JSON_VALUE_BOOLEAN;
}

bool json_is_number(json_value_t* json) {
    if(json == NULL)
        return false;
    return json->type == JSON_VALUE_NUMBER;
}

bool json_is_string(json_value_t* json) {
    if(json == NULL)
        return false;
    return json->type == JSON_VALUE_STRING;
}

bool json_is_object(json_value_t* json) {
    if(json == NULL)
        return false;
    return json->type == JSON_VALUE_OBJECT;
}

bool json_is_array(json_value_t* json) {
    if(json == NULL)
        return false;
    return json->type == JSON_VALUE_ARRAY;
}

char* json_dumps_append_double(char* prev, double val) {
    char* prevstr = prev != NULL ? prev : ""; 
    char* next = malloc(snprintf(NULL, 0, "%s%.8g", prevstr, val) + 1);
    sprintf(next, "%s%.8g", prevstr, val);
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

long json_est_size(json_value_t* json, long abort_after) {

    switch(json->type) {
        case JSON_VALUE_BOOLEAN: return json->as.boolean ? 4 : 5;
        case JSON_VALUE_NUMBER:  return snprintf(NULL, 0, "%.8g", json->as.number);
        case JSON_VALUE_NULL:    return 4;
        case JSON_VALUE_STRING:  return json->as.string.length;
        case JSON_VALUE_OBJECT: {
            long est = 2;
            json_object_t jobj = json->as.object;
            for(long i = 0; i < jobj.size; i++) {
                est += json_est_size(jobj.keys[i], abort_after-est);
                est += json_est_size(jobj.values[i], abort_after-est);
                if(est > abort_after)
                    return est;
            }
            return est;
        } break;
        case JSON_VALUE_ARRAY: {
            long est = 2;
            json_array_t jarr = json->as.array;
            for(long i = 0; i < jarr.size; i++) {
                est += json_est_size(jarr.values[i], abort_after-est);
                if(est > abort_after)
                    return est;
            }
            return est;
        } break;
        default:  return 100;
    }
}

char* json_dumps_append_boolean(char* prev, bool value) {
    return value
        ? json_dumps_append_string(prev, "true")
        : json_dumps_append_string(prev, "false");
}

char* json_dumps_append_quoted_string(char* prev, char* val, long vallen) {
    char* str = json_dumps_append_string(prev, "\"");
    str = json_dumps_append_lenstring(str, val, vallen);
    return json_dumps_append_string(str, "\"");
}

char* json_internal_dumps(json_value_t* json, int level, int indent);

char* json_dumps_append_array(char* prev, json_value_t* json, int level, int indent) {

    char* str = json_dumps_append_string(prev, "[");

    ptrdiff_t size = json->as.array.size;

    bool should_inline = json_est_size(json, 64) < 64;

    for(int i = 0; i < size; i++) {

        if(should_inline)
            str = json_dumps_append_string(str, " ");
        else
            str = json_apply_indent(str, level + 1, indent);

        char* inner = json_internal_dumps(
            json->as.array.values[i],
            level + 1,
            indent);

        str = json_dumps_append_string(str, inner);

        free(inner);

        if( i < (size - 1) )
            str = json_dumps_append_string(str, ",");
    }

    if( size > 0 && should_inline == false )
        str = json_apply_indent(str, level, indent);
    else
        str = json_dumps_append_string(str, " ");

    str = json_dumps_append_string(str, "]");

    return str;
}

char* json_dumps_append_object(char* prev, json_value_t* json, int level, int indent) {

    char* str = json_dumps_append_string(prev, "{");

    ptrdiff_t size = json->as.object.size;

    bool should_inline = json_est_size(json, 64) < 64;

    for(int i = 0; i < size; i++) {

        if(should_inline)
            str = json_dumps_append_string(str, " ");
        else
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
            level + 1,
            indent);

        str = json_dumps_append_string(str, inner);

        free(inner);

        if( i < (size - 1) )
            str = json_dumps_append_string(str, ",");
    }

    if( should_inline == false && size > 0 )
        str = json_apply_indent(str, level, indent);
    else
        str = json_dumps_append_string(str, " ");

    str = json_dumps_append_string(str, "}");

    assert(str[strlen(str)-1] == '}');

    return str;
}

char* json_dumps_append_error(char* prev, json_error_t error) {
    char* str = json_dumps_append_string(prev, "< expected '");
    str = json_dumps_append_string(str, json_tt_to_string(error.expected));
    str = json_dumps_append_string(str, "' but got ");
    if( error.got.buffer != NULL ) {
        str = json_dumps_append_string(str, "'");
        str = json_dumps_append_lenstring(str,
            error.got.buffer + error.got.start,
            (int)error.got.length);
        str = json_dumps_append_string(str, "'");
    } else {
        str = json_dumps_append_string(str, "null-pointer");
    }
    str = json_dumps_append_string(str, " >");
    return str;
}

char* json_internal_dumps(json_value_t* json, int level, int indent) {
    switch(json->type) {
        case JSON_VALUE_NULL:    return json_dumps_append_string(NULL, "null");
        case JSON_VALUE_NUMBER:  return json_dumps_append_double(NULL, json->as.number);
        case JSON_VALUE_BOOLEAN: return json_dumps_append_boolean(NULL, json->as.boolean);
        case JSON_VALUE_STRING:  return json_dumps_append_quoted_string(NULL, json->as.string.text, json->as.string.length);
        case JSON_VALUE_ERROR:   return json_dumps_append_error(NULL, json->as.error);
        case JSON_VALUE_ARRAY:   return json_dumps_append_array(NULL, json, level, indent);
        case JSON_VALUE_OBJECT:  return json_dumps_append_object(NULL, json, level, indent);
        default: {
            char* str = json_dumps_append_string(NULL, "< unknown type: ");
            str = json_dumps_append_integer(str, (int) json->type);
            str = json_dumps_append_string(str, " >");
            return str;
        };
    }
}

char* json_dumps(json_value_t* json, int indent) {
    return json_internal_dumps(json, 1, indent);
}

char* json_tt_to_string(json_tt_t tt) {
    switch(tt) {
        case JTT_NULL:              return "JTT_NULL";
        case JTT_NUMBER:            return "JTT_NUMBER";
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
        *type_out = JTT_NUMBER;
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
    char* tokenstr = tok.buffer + tok.start_offset;
    ptrdiff_t tokenlen = tok.length;

    switch(tok.type) {
        case JTT_BOOL: {
            json_value_t* result = NULL;
            if( pt_str_eq(tokenstr, tokenlen, "true") )
                result = json_boolean(allocator, true);
            else
                result = json_boolean(allocator, false);
            pt_state_advance(state);
            return result;
        } break;
        case JTT_NUMBER: {
            json_value_t* result = json_number(allocator, strtod(tokenstr, NULL));
            pt_state_advance(state);
            return result;
        } break;
        case JTT_STRING: {
            assert(tokenlen >= 2);
            json_value_t* result = json_string(allocator, tokenstr+1, tokenlen-2);
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
                bool ok = json_array_append(result, inner);
                assert(ok);
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