#include "shared/sh_json.h"
#include "shared/sh_parse_tools.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

json_value_t* json_value(json_value_type_t type) {
    json_value_t* val = malloc(sizeof(json_value_t));
    if( val == NULL )
        return NULL;
    val->type = type;
    return val;
}

json_value_t* json_string(char* value, ptrdiff_t len) {
    json_value_t* val = json_value(JSON_VALUE_STRING);
    if( val == NULL )
        return NULL;
    val->u.string.text = malloc((len + 1) * sizeof(char));
    if( val->u.string.text == NULL ) {
        free(val);
        return NULL;
    }
    len = strnlen(value, len);
    strncpy(val->u.string.text, value, len+1);
    val->u.string.text[len] = '\0';
    val->u.string.length = len;
    return val;
}

json_value_t* json_const_string(const char* value) {
    return json_string((char*)value, strlen(value));
}

json_value_t* json_null(void) {
    return json_value(JSON_VALUE_NULL);
}

json_value_t* json_boolean(bool value) {
    json_value_t* val = json_value(JSON_VALUE_BOOLEAN);
    if( val == NULL )
        return NULL;
    val->u.boolean = value;
    return val;
}

json_value_t* json_number_double(double value) {
    json_value_t* val = json_value(JSON_VALUE_NUMBER_DOUBLE);
    if( val == NULL )
        return NULL;
    val->u.number_double = value;
    return val;
}

json_value_t* json_number_integer(long value) {
    json_value_t* val = json_value(JSON_VALUE_NUMBER_INTEGER);
    if( val == NULL )
        return NULL;
    val->u.number_integer = value;
    return val;
}

json_value_t* json_error(json_tt_t expected, pt_token_t token) {
    json_value_t* val = json_value(JSON_VALUE_ERROR);
    if( val == NULL )
        return NULL;
    val->u.error.got.text   = token.text;
    val->u.error.got.length = token.length;
    val->u.error.expected   = expected;
    return val;
}

json_value_t* json_array(ptrdiff_t capacity) {
    json_value_t* val = json_value(JSON_VALUE_ARRAY);
    if( val == NULL )
        return NULL;
    json_value_t** content = (json_value_t**) malloc( sizeof(json_value_t*) * capacity );
    if( content == NULL ) {
        free(val);
        return NULL;
    }
    val->u.array.capacity = capacity;
    val->u.array.size = 0;
    val->u.array.values = content;
    return val;
}

bool json_array_append(json_value_t* json_array, json_value_t* value, bool free_arg_on_fail) {

    if( value == NULL )
        return false;

    if( json_array == NULL )
        goto free_arg_value_on_failure;

    if( json_array->type != JSON_VALUE_ARRAY )
        goto free_arg_value_on_failure;

    json_array_t* array = &json_array->u.array;
    if( array->capacity <= array->size ) {
        
        assert(array->capacity > 0);
        assert(array->size >= 0);

        ptrdiff_t newcap = array->capacity * 2;
        json_value_t** content = (json_value_t**) realloc(
            array->values,
            sizeof(json_value_t*) * newcap);

        if( content == NULL )
            goto free_arg_value_on_failure;

        array->capacity = newcap;
        array->values = content;
    }

    array->values[array->size] = value;
    array->size += 1;

    return true;

free_arg_value_on_failure:

    if(free_arg_on_fail)
        json_free(value);

    return false;
}

json_value_t* json_object(ptrdiff_t capacity) {

    json_value_t* val = json_value(JSON_VALUE_OBJECT);
    if( val == NULL )
        return NULL;

    json_value_t** values = (json_value_t**) malloc( sizeof(json_value_t*) * capacity );
    if( values == NULL ) {
        free(val);
        return NULL;
    }

    json_value_t** keys = (json_value_t**) malloc( sizeof(json_value_t*) * capacity );
    if( keys == NULL ) {
        free(values);
        free(val);
        return NULL;
    }

    val->u.object.capacity = capacity;
    val->u.object.keys = keys;
    val->u.object.values = values;
    val->u.object.size = 0;
    
    return val; 
}

bool json_object_append(json_value_t* json_object, json_value_t* key, json_value_t* value, bool free_on_fail) {

    if( json_object == NULL )
        goto free_args_on_failure;

    if( json_object->type != JSON_VALUE_OBJECT )
        goto free_args_on_failure;

    if( key == NULL || value == NULL )
        goto free_args_on_failure;

    json_object_t* object = &json_object->u.object;
    
    if( object->capacity <= object->size ) {
        assert(object->capacity > 0);
        assert(object->size >= 0);
        ptrdiff_t newcap = object->capacity * 2;
        json_value_t** values = (json_value_t**) realloc(
            object->values,
            sizeof(json_value_t*) * newcap);
        if( values == NULL )
            goto free_args_on_failure;
        json_value_t** keys = (json_value_t**) realloc(
            object->keys,
            sizeof(json_value_t*) * newcap);
        if( keys == NULL ) {
            free(values);
            goto free_args_on_failure;
        }
        object->capacity = newcap;
        object->values = values;
        object->keys = keys;
    }

    object->values[object->size] = value;
    object->keys[object->size] = key;
    object->size += 1;

    return true;

free_args_on_failure:

    if(free_on_fail) {
        json_free(key);
        json_free(value);
    }

    return false;
}

bool json_string_value_equals(json_value_t* a, json_value_t* b) {
    if( a->type != JSON_VALUE_STRING )
        return false;
    if( b->type != a->type )
        return false;
    if(a->u.string.length != b->u.string.length)
        return false;
    if(a->u.string.text == b->u.string.text)
        return true;
    ptrdiff_t len = a->u.string.length;
    for(ptrdiff_t i = 0; i < len; i++) {
        if( a->u.string.text[i] != b->u.string.text[i] )
            return false;
    }
    return true;
}

bool json_object_set(json_value_t* json_object, json_value_t* key, json_value_t* value, bool free_args_on_fail) {

    if( json_object == NULL || key == NULL || value == NULL ) {
        if( free_args_on_fail ) {
            json_free(key);
            json_free(value);
        }
        return false;
    }

    json_object_t* obj = &json_object->u.object;
    for(int i = 0; i < obj->size; i++) {
        if( json_string_value_equals(obj->keys[i], key) ) {
            if( obj->values[i] != NULL )
                free(obj->values[i]);
            obj->values[i] = value;
            return true;
        }
    }

    return json_object_append(json_object, key, value, free_args_on_fail);
}

json_value_t* json_object_get(json_value_t* json_object, char* key, ptrdiff_t key_len) {
    if( key == NULL || json_object == NULL )
        return NULL;
    json_object_t* obj = &json_object->u.object;
    for(int i = 0; i < obj->size; i++) {
        if( strncmp(key, obj->keys[i]->u.string.text, key_len) == 0 )
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
            return json_dumps_append_double(NULL, json->u.number_double);
        case JSON_VALUE_NUMBER_INTEGER:
            return json_dumps_append_integer(NULL, json->u.number_integer);
        case JSON_VALUE_STRING: {
            char* str = json_dumps_append_string(NULL, "\"");
            str = json_dumps_append_string(str, json->u.string.text);
            str = json_dumps_append_string(str, "\"");
            return str;
        } break;
        case JSON_VALUE_BOOLEAN: {
            if(json->u.boolean)
                return json_dumps_append_string(NULL, "true");
            else
                return json_dumps_append_string(NULL, "false");
        } break;
        case JSON_VALUE_ARRAY: {
            char* str = json_dumps_append_string(NULL, "[");
            ptrdiff_t size = json->u.array.size;
            for(int i = 0; i < size; i++) {
                str = json_apply_indent(str, level + 1, indent);
                char* inner = json_internal_dumps(
                    json->u.array.values[i],
                    level + 2,
                    indent);
                str = json_dumps_append_string(str, inner);
                free(inner);
                if( i < (size - 1) )
                    str = json_dumps_append_string(str, ",");
            }
            if( size > 0 )
                str = json_apply_indent(str, level, indent);
            str = json_dumps_append_string(str, "]");
            return str;
        } break;
        case JSON_VALUE_OBJECT: {
            char* str = json_dumps_append_string(NULL, "{");
            ptrdiff_t size = json->u.object.size;
            for(int i = 0; i < size; i++) {
                str = json_apply_indent(str, level + 1, indent);
                char* keystr = json_internal_dumps(
                    json->u.object.keys[i], 0, 0);
                str = json_dumps_append_string(str, keystr);
                free(keystr);
                str = json_dumps_append_string(str, ":");
                if( indent > 0 )
                    str = json_dumps_append_string(str, " ");
                char* inner = json_internal_dumps(
                    json->u.object.values[i],
                    level + 2,
                    indent);
                str = json_dumps_append_string(str, inner);
                free(inner);
                if( i < (size - 1) )
                    str = json_dumps_append_string(str, ",");
            }
            if( size > 0 )
                str = json_apply_indent(str, level, indent);
            str = json_dumps_append_string(str, "}");
            return str;
        } break;
        case JSON_VALUE_ERROR: {
            char* str = json_dumps_append_string(NULL, "<error: expected '");
            str = json_dumps_append_string(str, json_tt_to_string(json->u.error.expected));
            str = json_dumps_append_string(str, "' but got ");
            if( json->u.error.got.text != NULL ) {
                str = json_dumps_append_string(str, "'");
                str = json_dumps_append_lenstring(str,
                    json->u.error.got.text,
                    (int)json->u.error.got.length);
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

void json_free(json_value_t* json) {
    if(json == NULL)
        return;
    switch(json->type) {
        case JSON_VALUE_BOOLEAN:
        case JSON_VALUE_NUMBER_DOUBLE:
        case JSON_VALUE_NUMBER_INTEGER:
        case JSON_VALUE_NULL:
        case JSON_VALUE_ERROR:
            *json = (json_value_t) { 0 };
            free(json);
            break;
        case JSON_VALUE_STRING: {
            if( json->u.string.text != NULL )
                free(json->u.string.text);
            *json = (json_value_t) { 0 };
            free(json);
        } break;
        case JSON_VALUE_ARRAY: {
            for(int i = 0; i < json->u.array.size; i++) {
                json_free(json->u.array.values[i]);
            }
            free(json->u.array.values);
            *json = (json_value_t) { 0 };
            free(json);
        } break;
        case JSON_VALUE_OBJECT: {
            for(int i = 0; i < json->u.object.size; i++) {
                json_free(json->u.object.keys[i]);
                json_free(json->u.object.values[i]);
            }
            free(json->u.object.values);
            free(json->u.object.keys);
            *json = (json_value_t) { 0 };
            free(json);
        } break;
        default:
            // Free?
            assert(false);
            break;
    }
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

json_value_t* json_parse_value(pt_state_t* state) {

    //todo: out of memory handling

    pt_token_t tok = pt_state_peek(state, 0);

    switch(tok.type) {
        case JTT_BOOL: {
            json_value_t* result = NULL;
            if( pt_str_eq(tok.text, tok.length, "true") )
                result = json_boolean(true);
            else
                result = json_boolean(false);
            pt_state_advance(state);
            return result;
        } break;
        case JTT_NUMBER_DOUBLE: {
            json_value_t* result = json_number_double(strtod(tok.text, NULL));
            pt_state_advance(state);
            return result;
        } break;
        case JTT_NUMBER_INTEGER: {
            json_value_t* result = json_number_integer((long) strtod(tok.text, NULL));
            pt_state_advance(state);
            return result;
        } break;
        case JTT_STRING: {
            assert(tok.length >= 2);
            json_value_t* result = json_string(tok.text+1, tok.length-2);
            pt_state_advance(state);
            return result;
        } break;
        case JTT_NULL: {
            pt_state_advance(state);
            return json_null();
        } break;
        case JTT_ARRAY_OPEN: {
            json_value_t* result = json_array(8);

            pt_state_advance(state);

            while( pt_state_has_tokens(state) ) {

                if(pt_state_advance_if(state, JTT_ARRAY_CLOSE))
                    break;

                json_value_t* inner = json_parse_value(state);
                json_array_append(result, inner, true);
                pt_state_advance_if(state, JTT_SEPARATOR);
            }

            return result;
        } break;
        case JTT_OBJECT_OPEN: {

            json_value_t* result = json_object(8);

            pt_state_advance(state);

            while( pt_state_has_tokens(state) ) {

                if( pt_state_advance_if(state, JTT_OBJECT_CLOSE) )
                    break;

                json_value_t* key = NULL;
                if( pt_state_match_any(state, JTT_STRING) )
                    key = json_parse_value(state);
                else {
                    key = json_error(JTT_STRING, pt_state_peek(state, 0));
                    pt_state_advance(state);
                }

                pt_state_advance_if(state, JTT_KVP_SEPARATOR);

                json_value_t* value = json_parse_value(state);
                json_object_append(result, key, value, true);

                pt_state_advance_if(state, JTT_SEPARATOR);
            }

            return result;
        } break;
    }

    
    json_value_t* err = json_error(JTT_UNKNOWN, tok);
    pt_state_advance(state);
    return err;
}

json_value_t* json_parse(char* json_str, ptrdiff_t len) {
    
    pt_state_t p = (pt_state_t){ 0 };

    pt_result_code_t res = pt_state_init(&p, json_scan, json_str, len);

    if( res > 0 )
        return NULL; // error | nothing parsed

    json_value_t* result = json_parse_value(&p);
    
    pt_state_free(&p);

    return result;
}