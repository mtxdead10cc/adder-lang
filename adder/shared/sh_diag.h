#ifndef SH_DIAG_H__
#define SH_DIAG_H__

#include "shared/sh_src.h"
#include "shared/sh_arena.h"
#include "shared/sh_macros.h"

typedef enum diword_t {

    _NONE_             = 0,

    _UNEXPECTED_       = 1,
    _UNDEFINED_        = 2,
    _REDEFINED_        = 3,
    _INSUFFICIENT_     = 4,
    _MISSING_          = 5,
    _MISMATCHING_      = 6,

    _TOKEN_,
    _KEYWORD_,
    _BOOLEAN_,
    _CHARACTER_,
    _NUMBER_,
    _STRING_,
    _ARRAY_,
    _BINARY_,
    _UNARY_,
    _OPERATOR_,
    _OPERAND_,
    _ARGUMENT_,
    _RETURN_,
    _FUNCTION_,
    _VARIABLE_,
    _DEFINITION_,
    _TYPE_,
    _COUNT_,
    _NAME_,
    _HOST_,
    _MEMORY_,
    _PROGRAM_,
    _ENTRY_,
    _POINT_,

} diword_t;

typedef uint64_t diphrase_t;

typedef enum dimsg_tag_t {
    DIMSG_SRCREF_TEXT,
    DIMSG_SRCREF_FULL,
    DIMSG_STRING,
} dimsg_tag_t;

typedef struct dimsg_t {
    dimsg_tag_t tag;
    union {
        srcref_t srcref;
        char*    string;
    } as;
} dimsg_t;

#define diag_str(STR)     (dimsg_t) { .tag = DIMSG_STRING, .as.string = (STR) }
#define diag_reftext(REF) (dimsg_t) { .tag = DIMSG_SRCREF_TEXT, .as.srcref = (REF) }
#define diag_reffull(REF) (dimsg_t) { .tag = DIMSG_SRCREF_FULL, .as.srcref = (REF) }

// UNEXPECTED   TOKEN       TYPE
// UNEXPECTED   OPERAND     TYPE
// UNKNOWN      CHARACTER
// MISSING      PROGRAM     ENTRY   POINT
// INSUFFICIENT HOST        MEMORY
// UNDEFINED    VARIABLE    NAME 
// UNDEFINED    FUNCTION    NAME
// MISMATCHING  ARGUMENT    COUNT
// REDEFINED    VARIABLE    NAME

typedef enum diag_kind_t {
    DIAG_ERROR,
    DIAG_WARNING,
    DIAG_INFO
} diag_kind_t;

typedef struct diag_t diag_t;
typedef struct diag_t {
    diag_kind_t kind;
    diphrase_t  phrase;
    diag_t*     next;
    size_t      size;
    dimsg_t     message[];
} diag_t;


struct diag_phrase_params {
    int _sentinel;
    diword_t w0; diword_t w1;
    diword_t w2; diword_t w3;
    diword_t w4; diword_t w5;
    diword_t w6; diword_t w7;
};

inline static diphrase_t _diag_make_phrase(struct diag_phrase_params *args) {
    return ( (((diphrase_t)(args->w0) & 0xFF) << 8*7)
           | (((diphrase_t)(args->w1) & 0xFF) << 8*6)
           | (((diphrase_t)(args->w2) & 0xFF) << 8*5)
           | (((diphrase_t)(args->w3) & 0xFF) << 8*4)
           | (((diphrase_t)(args->w4) & 0xFF) << 8*3)
           | (((diphrase_t)(args->w5) & 0xFF) << 8*2)
           | (((diphrase_t)(args->w6) & 0xFF) << 8*1)
           | (((diphrase_t)(args->w7) & 0xFF) << 8*0) );
}

#define diag_phrase(...) _diag_make_phrase(\
    &((struct diag_phrase_params){ ._sentinel = 0, __VA_ARGS__ }))


inline static diag_t* mk_diag(arena_t* arena, diag_kind_t kind, diphrase_t phrase, dimsg_t* message, size_t size) {
    diag_t* d = (diag_t*) aalloc(arena, sizeof(diag_t) + (sizeof(dimsg_t) * size));
    if( d == NULL )
        return NULL;
    d->kind = kind;
    d->phrase = phrase;
    d->next = NULL;
    d->size = size;
    memcpy(d->message, message, sizeof(dimsg_t) * size);
    return d;
}

#define diag_error(ARENA, PHRASE, ...) mk_diag( \
	(ARENA), DIAG_ERROR, (PHRASE),     \
    VA_ARRAY(dimsg_t, __VA_ARGS__),    \
    VA_ARRAYLEN(dimsg_t, __VA_ARGS__))

#define diag_warning(ARENA, PHRASE, ...) mk_diag( \
	(ARENA), DIAG_WARNING, (PHRASE),     \
    VA_ARRAY(dimsg_t, __VA_ARGS__),    \
    VA_ARRAYLEN(dimsg_t, __VA_ARGS__))

inline static const char* diag_word_to_string(diword_t w) {
    switch(w) {
        case _NONE_:           return "";
        case _UNEXPECTED_:     return "unexpected";
        case _UNDEFINED_:      return "undefined";
        case _REDEFINED_:      return "redefined";
        case _INSUFFICIENT_:   return "insufficient";
        case _MISSING_:        return "missing";
        case _MISMATCHING_:    return "mismatching";
        case _TOKEN_:          return "token";
        case _KEYWORD_:        return "keyword";
        case _BOOLEAN_:        return "boolean";
        case _CHARACTER_:      return "character";
        case _NUMBER_:         return "number";
        case _STRING_:         return "string";
        case _ARRAY_:          return "array";
        case _BINARY_:         return "binary";
        case _UNARY_:          return "unary";
        case _OPERATOR_:       return "operator";
        case _OPERAND_:        return "operand";
        case _ARGUMENT_:       return "argument";
        case _RETURN_:         return "return";
        case _FUNCTION_:       return "function";
        case _VARIABLE_:       return "variable";
        case _DEFINITION_:     return "definition";
        case _TYPE_:           return "type";
        case _COUNT_:          return "count";
        case _NAME_:           return "name";
        case _HOST_:           return "host";
        case _MEMORY_:         return "memory";
        case _PROGRAM_:        return "program";
        case _ENTRY_:          return "entry";
        case _POINT_:          return "point";
        default:               return "<unknown diagnostic id>";
    }
}

inline static size_t diag_message_to_string(dimsg_t* msg, cstr_t* str) {
    switch(msg->tag) {
        case DIMSG_STRING: {
            int printed = cstr_append_fmt(str, "%s", msg->as.string);
            if(printed < 0)
                return 0;
            return (size_t) printed;
        };
        case DIMSG_SRCREF_TEXT: {
            int printed = cstr_append_fmt(str, "%.*s",
                srcref_len(msg->as.srcref),
                srcref_ptr(msg->as.srcref));
            if(printed < 0)
                return 0;
            return (size_t) printed;
        };
        case DIMSG_SRCREF_FULL: {
            srcloc_t loc = srcref_get_location(msg->as.srcref);
            int printed = cstr_append_fmt(str, "%.*s %.*s:%lu:%lu",
                srcref_len(msg->as.srcref),
                srcref_ptr(msg->as.srcref),
                (int) loc.path_length, loc.path,
                loc.line,
                loc.column);
            if(printed < 0)
                return 0;
            return (size_t) printed;
        };
        default: {
            return 0;
        };
    }
}

inline static const char* diag_kind_to_string(diag_t* diag) {
    switch(diag->kind) {
        case DIAG_ERROR:   return "\033[0;31mERROR\033[0m";
        case DIAG_WARNING: return "\033[0;33mWARNING\033[0m";
        case DIAG_INFO:    return "\033[0;33mINFO\033[0m";
        default: return "<UNKNOWN-TAG>";
    }
}

inline static size_t diag_to_string(diag_t* diag, cstr_t* str) {

    size_t len = 0;

    const char* kind_str = diag_kind_to_string(diag);
    int wn = cstr_append_fmt(str, "%s: ", kind_str);
    if( wn > 0 )
        len += (size_t) wn;

    for(int i = 7; i >= 0; i--) {
        diword_t word_id = (diag->phrase >> (i * 8)) & 0xFF;
        if( word_id == _NONE_ )
            break;
        const char* word_str = diag_word_to_string(word_id);
        wn = cstr_append_fmt(str, "%s ", word_str);
        if( wn > 0 )
            len += (size_t) wn;
    }

    for(size_t i = 0; i < diag->size; i++) {
        len += diag_message_to_string(&diag->message[i], str);
    }

    return len;
}

/*
    expected <a> to have type <A> but <a> is of type <B>
    a: ref | ast
    A: type | token-type
    B: type | token-type

    <a> could not be understood, did you mean any of the following [proposals] 
    <a> is used as a <B>, but <a> is of type <A>
    <a> of type <A> has not been defined yet
    <a> of type <A> is already defined here <a>
    the program has no entrypoints, you can either ...
    <a> of type <A> cannot be defined within a function (<b>)
    <a> of type <A> must be defined inside a function

    host error 	ran out of host system memory
    internal error 	unexpected state
*/

#endif // SH_DIAG_H__