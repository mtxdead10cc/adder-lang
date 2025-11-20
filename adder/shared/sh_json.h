#ifndef SH_JSON_H_
#define SH_JSON_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum json_value_type_t {
    JSON_VALUE_NULL,
    JSON_VALUE_STRING,
    JSON_VALUE_NUMBER_DOUBLE,
    JSON_VALUE_NUMBER_INTEGER,
    JSON_VALUE_BOOLEAN,
    JSON_VALUE_ARRAY,
    JSON_VALUE_OBJECT,
    JSON_VALUE_ERROR
} json_value_type_t;

typedef enum json_tt_t {
    JTT_UNKNOWN         = 0x0000,
    JTT_NULL            = 0x0001,
    JTT_NUMBER_INTEGER  = 0x0002,
    JTT_NUMBER_DOUBLE   = 0x0004,
    JTT_STRING          = 0x0008,
    JTT_BOOL            = 0x0010,
    JTT_SEPARATOR       = 0x0020,
    JTT_ARRAY_OPEN      = 0x0040,
    JTT_ARRAY_CLOSE     = 0x0080,
    JTT_OBJECT_OPEN     = 0x0100,
    JTT_OBJECT_CLOSE    = 0x0200,
    JTT_KVP_SEPARATOR   = 0x0400
} json_tt_t;

typedef struct json_value_t json_value_t;

typedef struct json_object_t {
    ptrdiff_t       size;
    ptrdiff_t       capacity;
    json_value_t**  keys;
    json_value_t**  values;
} json_object_t;

typedef struct json_array_t {
    ptrdiff_t       size;
    ptrdiff_t       capacity;
    json_value_t**  values;
} json_array_t;

typedef struct json_string_t {
    char*       text;
    ptrdiff_t   length;
} json_string_t;

typedef struct json_error_t {
    json_tt_t         expected;
    struct {
        char*         text;
        ptrdiff_t     length;
    } got;
} json_error_t;

typedef struct json_value_t {
    json_value_type_t type;
    union {
        json_object_t object;
        json_array_t  array;
        json_string_t string;
        json_error_t  error;
        double        number_double;
        long          number_integer;
        bool          boolean;
    } u;
} json_value_t;

char* json_tt_to_string(json_tt_t tt);

json_value_t*   json_null(void);
json_value_t*   json_string(char* value, ptrdiff_t len);
json_value_t*   json_const_string(const char* value);
json_value_t*   json_boolean(bool value);
json_value_t*   json_number_double(double value);
json_value_t*   json_number_integer(long value);

json_value_t*   json_array(ptrdiff_t capacity);
bool            json_array_append(json_value_t* json_array, json_value_t* value, bool free_arg_on_fail);

json_value_t*   json_object(ptrdiff_t capacity);
bool            json_object_set(json_value_t* json_object, json_value_t* key, json_value_t* value, bool free_args_on_fail);
json_value_t*   json_object_get(json_value_t* json_object, char* key, ptrdiff_t key_len);
json_value_t* json_object_get_const(json_value_t* json_object, const char* key);

void            json_free(json_value_t* json);

char*           json_dumps(json_value_t* json, int indent);
json_value_t*   json_parse(char* json_str, ptrdiff_t len);

#endif // SH_JSON_H_