#ifndef SH_JSON_H_
#define SH_JSON_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum json_value_type_t {
    JSON_VALUE_NULL,
    JSON_VALUE_STRING,
    JSON_VALUE_NUMBER,
    JSON_VALUE_BOOLEAN,
    JSON_VALUE_ARRAY,
    JSON_VALUE_OBJECT,
    JSON_VALUE_ERROR
} json_value_type_t;

typedef enum json_tt_t {
    JTT_UNKNOWN         = 0x0000,
    JTT_NULL            = 0x0001,
    JTT_NUMBER          = 0x0002,
    JTT_STRING          = 0x0004,
    JTT_BOOL            = 0x0008,
    JTT_SEPARATOR       = 0x0010,
    JTT_ARRAY_OPEN      = 0x0020,
    JTT_ARRAY_CLOSE     = 0x0040,
    JTT_OBJECT_OPEN     = 0x0080,
    JTT_OBJECT_CLOSE    = 0x0100,
    JTT_KVP_SEPARATOR   = 0x0200
} json_tt_t;

typedef struct json_value_t json_value_t;
typedef struct arena_t arena_t;

typedef struct json_object_t {
    ptrdiff_t       size;
    ptrdiff_t       capacity;
    arena_t*        allocator;
    json_value_t**  keys;
    json_value_t**  values;
} json_object_t;

typedef struct json_array_t {
    ptrdiff_t       size;
    ptrdiff_t       capacity;
    arena_t*        allocator;
    json_value_t**  values;
} json_array_t;

typedef struct json_string_t {
    char*           text;
    ptrdiff_t       length;
} json_string_t;

typedef struct json_error_t {
    json_tt_t   expected;
    struct {
        char*   buffer;
        size_t  start;
        size_t  length;
    } got;
} json_error_t;

typedef struct json_value_t {
    json_value_type_t type;
    union {
        json_object_t object;
        json_array_t  array;
        json_string_t string;
        json_error_t  error;
        double        number;
        bool          boolean;
    } as;
} json_value_t;

char* json_tt_to_string(json_tt_t tt);

json_value_t*   json_null(arena_t* allocator);
json_value_t*   json_string(arena_t* allocator, char* value, ptrdiff_t len);
json_value_t*   json_const_string(arena_t* allocator, const char* value);
json_value_t*   json_boolean(arena_t* allocator, bool value);
json_value_t*   json_number(arena_t* allocator, double value);

bool json_is_bool(json_value_t* json);
bool json_is_number(json_value_t* json);
bool json_is_string(json_value_t* json);
bool json_is_object(json_value_t* json);
bool json_is_array(json_value_t* json);

ptrdiff_t       json_get_size(json_value_t* json);

json_value_t*   json_array(arena_t* allocator, ptrdiff_t capacity);
bool            json_array_append(json_value_t* json_array, json_value_t* value);
json_value_t*   json_array_get(json_value_t* array, ptrdiff_t index);

json_value_t*   json_object(arena_t* allocator, ptrdiff_t capacity);
bool            json_object_set(json_value_t* json_object, json_value_t* key, json_value_t* value);
json_value_t*   json_object_getn(json_value_t* json_object, char* key, ptrdiff_t key_len);
json_value_t*   json_object_get(json_value_t* json_object, const char* key);
bool            json_object_has(json_value_t* json_object, const char* key, json_value_type_t value_type);

char*           json_dumps(json_value_t* json, int indent);
json_value_t*   json_parse(arena_t* allocator, char* json_str, ptrdiff_t len);

#endif // SH_JSON_H_