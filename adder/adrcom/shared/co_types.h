#ifndef GVM_COMPILER_TYPES_H_
#define GVM_COMPILER_TYPES_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <shared/sh_types.h>
#include <shared/sh_src.h>

#define TRACE_MSG_MAX_LEN 256

typedef enum trace_msg_type_t {
    TM_NONE,
    TM_INFO,
    TM_WARNING,
    TM_ERROR,
    TM_OUT_OF_MEMORY,
    TM_INTERNAL_ERROR
} trace_msg_type_t;

typedef struct trace_msg_t {
    trace_msg_type_t type;
    char*       source_path;
    srcref_t    source_location;
    size_t      length;
    char        message[TRACE_MSG_MAX_LEN];
} trace_msg_t;

typedef struct trace_t {
    size_t       message_count;
    size_t       message_capacity;
    trace_msg_t* messages;
    size_t       error_count;
} trace_t;


#define LANG_TYPENAME_VOID      "void"
#define LANG_TYPENAME_BOOL      "bool"
#define LANG_TYPENAME_FLOAT     "float"
#define LANG_TYPENAME_INT       "int"
#define LANG_TYPENAME_STRING    "string"
#define LANG_TYPENAME_ARRAY     "array"
#define LANG_TYPENAME_CHAR      "char"

#endif // GVM_COMPILER_TYPES_H_
