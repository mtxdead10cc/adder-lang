#ifndef SH_DIAG_H__
#define SH_DIAG_H__

#include "shared/sh_src.h"
#include "shared/sh_arena.h"
#include "shared/sh_macros.h"

typedef enum diword_t {

    _NONE             = 0,
    _UNEXPECTED       = 1,
    _UNDEFINED        = 2,
    _REDEFINED        = 3,
    _INSUFFICIENT     = 4,
    _MISSING          = 5,
    _MISMATCHING      = 6,
    _TOKEN,
    _KEYWORD,
    _BOOLEAN,
    _CHARACTER,
    _NUMBER,
    _STRING,
    _ARRAY,
    _BINARY,
    _UNARY,
    _OPERATOR,
    _OPERAND,
    _ARGUMENT,
    _RETURN,
    _FUNCTION,
    _VARIABLE,
    _DEFINITION,
    _TYPE,
    _COUNT,
    _NAME,
    _HOST,
    _MEMORY,
    _PROGRAM,
    _ENTRY,
    _POINT,
    _END,
    _OF,
    _STREAM,
    _FORMAT,
    _STATEMENT,
    _SEQUENCE

} diword_t;

typedef uint64_t diphrase_t;

typedef enum dimsg_tag_t {
    DIMSG_SRCREF_TEXT,
    DIMSG_SRCREF_LOCATION,
    DIMSG_SRCREF_SECONDARY_LOCATION,
    DIMSG_STRING,
} dimsg_tag_t;

typedef struct dimsg_t {
    dimsg_tag_t tag;
    union {
        srcref_t srcref;
        char*    string;
    } as;
} dimsg_t;

#define diag_str(STR)               (dimsg_t) { .tag = DIMSG_STRING, .as.string = (STR) }
#define diag_refstr(REF)            (dimsg_t) { .tag = DIMSG_SRCREF_TEXT, .as.srcref = (REF) }
#define diag_refloc(REF)            (dimsg_t) { .tag = DIMSG_SRCREF_LOCATION, .as.srcref = (REF) }
#define diag_refloc_secondary(REF)  (dimsg_t) { .tag = DIMSG_SRCREF_SECONDARY_LOCATION, .as.srcref = (REF) }

inline static bool dimsg_is_primary_srcref(dimsg_t* m) {
    return m->tag == DIMSG_SRCREF_LOCATION
        || m->tag == DIMSG_SRCREF_TEXT;
}

inline static bool dimsg_is_any_srcref(dimsg_t* m) {
    return m->tag == DIMSG_SRCREF_LOCATION
        || m->tag == DIMSG_SRCREF_TEXT
        || m->tag == DIMSG_SRCREF_SECONDARY_LOCATION;
}

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
    DIAG_INFO,
    DIAG_WARNING,
    DIAG_ERROR
} diag_kind_t;

typedef struct diag_t diag_t;
typedef struct diag_t {
    diag_kind_t kind;
    diphrase_t  phrase;
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
        case _NONE:           return "NONE";
        case _UNEXPECTED:     return "UNEXPECTED";
        case _UNDEFINED:      return "UNDEFINED";
        case _REDEFINED:      return "REDEFINED";
        case _INSUFFICIENT:   return "INSUFFICIENT";
        case _MISSING:        return "MISSING";
        case _MISMATCHING:    return "MISMATCHING";
        case _TOKEN:          return "TOKEN";
        case _KEYWORD:        return "KEYWORD";
        case _BOOLEAN:        return "BOOLEAN";
        case _CHARACTER:      return "CHARACTER";
        case _NUMBER:         return "NUMBER";
        case _STRING:         return "STRING";
        case _ARRAY:          return "ARRAY";
        case _BINARY:         return "BINARY";
        case _UNARY:          return "UNARY";
        case _OPERATOR:       return "OPERATOR";
        case _OPERAND:        return "OPERAND";
        case _ARGUMENT:       return "ARGUMENT";
        case _RETURN:         return "RETURN";
        case _FUNCTION:       return "FUNCTION";
        case _VARIABLE:       return "VARIABLE";
        case _DEFINITION:     return "DEFINITION";
        case _TYPE:           return "TYPE";
        case _COUNT:          return "COUNT";
        case _NAME:           return "NAME";
        case _HOST:           return "HOST";
        case _MEMORY:         return "MEMORY";
        case _PROGRAM:        return "PROGRAM";
        case _ENTRY:          return "ENTRY";
        case _POINT:          return "POINT";
        case _END:            return "END";
        case _OF:             return "OF";
        case _STREAM:         return "STREAM";
        case _FORMAT:         return "FORMAT";
        case _STATEMENT:      return "STATEMENT";
        case _SEQUENCE:       return "SEQUENCE";
        default:              return "<unknown diagnostic id>";
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
        case DIMSG_SRCREF_LOCATION: {
            srcloc_t loc = srcref_get_location(msg->as.srcref);
            int printed = cstr_append_fmt(str, "%.*s:%lu:%lu",
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

inline static srcref_t diag_collect_srcref(diag_t* diag) {
    srcref_t agg = {0};
    for(size_t i = 0; i < diag->size; i++) {
        if(dimsg_is_primary_srcref(&diag->message[i]) == false)
            continue;
        agg = srcref_combine(agg, diag->message[i].as.srcref);
    }
    return agg;
}

inline static size_t diag_to_string(diag_t* diag, cstr_t* str) {

    size_t len = 0;

    const char* kind_str = diag_kind_to_string(diag);
    int wn = cstr_append_fmt(str, "%s: ", kind_str);
    if( wn > 0 )
        len += (size_t) wn;

    for(int i = 7; i >= 0; i--) {
        diword_t word_id = (diag->phrase >> (i * 8)) & 0xFF;
        if( word_id == _NONE )
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