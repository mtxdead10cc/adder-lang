#ifndef TRACE_H_
#define TRACE_H_

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <sh_types.h>
#include <sh_utils.h>
#include <sh_ffi.h>
#include <sh_ift.h>
#include <sh_utils.h>
#include <sh_log.h>

#include "co_types.h"

inline static bool trace_init(trace_t* trace, size_t capacity) {
    trace_msg_t* messages = (trace_msg_t*) malloc( sizeof(trace_msg_t) * capacity );
    if( messages == NULL )
        return false;
    trace->messages = messages;
    trace->message_count = 0;
    trace->message_capacity = capacity;
    trace->current_source_path = NULL;
    trace->error_count = 0;
    return true;
}

inline static void trace_destroy(trace_t* trace) {
    if( trace->messages != NULL ) {
        free(trace->messages);
    }
    memset(trace, 0, sizeof(trace_t));
}

inline static void trace_set_current_source_path(trace_t* trace, char* path) {
    trace->current_source_path = path;
}

inline static srcref_t trace_no_ref(void) {
    return (srcref_t) { 0 };
}

inline static trace_msg_t* trace_create_message(trace_t* trace, trace_msg_type_t type, srcref_t src_loc) {
    if( (trace->message_count + 1) >= trace->message_capacity ) {
        size_t new_cap = trace->message_count * 2;
        trace_msg_t* messages = (trace_msg_t*) realloc(trace->messages, new_cap * sizeof(trace_msg_t));
        if( messages == NULL )
            return NULL;
        trace->messages = messages;
        trace->message_capacity = new_cap;
    }
    trace_msg_t* msg = &trace->messages[trace->message_count++];
    msg->type = type;
    if( type >= TM_ERROR )
        trace->error_count ++;
    msg->source_path = trace->current_source_path;
    msg->source_location = src_loc;
    msg->length = 0;
    return msg;
}

inline static void trace_out_of_memory_error(trace_t* trace) {
    if( trace->message_count + 1 < trace->message_capacity ) {
        trace_create_message(trace, TM_OUT_OF_MEMORY, trace_no_ref());
    } else {
        sh_log_error("out of system memory: overwriting last trace message.\n");
        trace->messages[trace->message_count].type = TM_OUT_OF_MEMORY;
        trace->messages[trace->message_count].length = 0;
        trace->error_count ++;
    }
}



inline static int trace_msg_append(trace_msg_t* msg, char* str, size_t slen) {
    if( msg == NULL )
        return 0;
    int remaining = (int) TRACE_MSG_MAX_LEN - (int) msg->length;
    if( remaining <= 0 || slen == 0 )
        return 0;
    int write_len = clamp((int)slen, 0, remaining);
    memcpy(msg->message + msg->length, str, write_len * sizeof(char));
    msg->length += write_len;
    return write_len;
}

inline static int trace_msg_append_fmt(trace_msg_t* msg, const char* fmt, ...) {
    if( msg == NULL )
        return 0;
    int remaining = (int) TRACE_MSG_MAX_LEN - (int) msg->length;
    if( remaining <= 0 )
        return 0;
    va_list args;
    va_start(args, fmt);
    int w = vsnprintf(msg->message + msg->length, remaining, fmt, args);
    msg->length += w;
    va_end(args);
    return w;
}

#define trace_msg_append_costr(MSG, STR) trace_msg_append((MSG), (STR), (sizeof((STR))/sizeof(char)) - 1)

inline static int trace_msg_append_token_type_name(trace_msg_t* msg, token_type_t type) {
    switch (type) {
        case TT_INITIAL:        return trace_msg_append_costr(msg, "initial token");
        case TT_SPACE:          return trace_msg_append_costr(msg, "space");
        case TT_COMMENT:        return trace_msg_append_costr(msg, "comment");
        case TT_SYMBOL:         return trace_msg_append_costr(msg, "symbol");
        case TT_NUMBER:         return trace_msg_append_costr(msg, "number");
        case TT_BOOLEAN:        return trace_msg_append_costr(msg, "bool");
        case TT_STRING:         return trace_msg_append_costr(msg, "string");
        case TT_SEPARATOR:      return trace_msg_append_costr(msg, "separator");
        case TT_STATEMENT_END:  return trace_msg_append_costr(msg, "end of statement");
        case TT_ARROW:          return trace_msg_append_costr(msg, "function return type arrow");
        case TT_ASSIGN:         return trace_msg_append_costr(msg, "assignment");
        case TT_CMP_LT:         return trace_msg_append_costr(msg, "opening angle bracket alt. less than");
        case TT_CMP_GT:         return trace_msg_append_costr(msg, "closing angle bracket alt. greater than");
        case TT_OPEN_PAREN:     return trace_msg_append_costr(msg, "left parenthesis");
        case TT_CLOSE_PAREN:    return trace_msg_append_costr(msg, "right parenthesis");
        case TT_OPEN_CURLY:     return trace_msg_append_costr(msg, "opening brace");
        case TT_CLOSE_CURLY:    return trace_msg_append_costr(msg, "closing brace");
        case TT_OPEN_SBRACKET:  return trace_msg_append_costr(msg, "opening square bracket");
        case TT_CLOSE_SBRACKET: return trace_msg_append_costr(msg, "closing square bracket");
        case TT_BINOP_AND:      return trace_msg_append_costr(msg, "binary and-operator");
        case TT_BINOP_OR:       return trace_msg_append_costr(msg, "binary or-operator");
        case TT_UNOP_NOT:       return trace_msg_append_costr(msg, "unary not-operator");
        case TT_FINAL:          return trace_msg_append_costr(msg, "final token");
        case TT_NOTHING:        return trace_msg_append_costr(msg, "nothing");
        case TT_BINOP_MUL:      return trace_msg_append_costr(msg, "multiply operator");
        case TT_BINOP_DIV:      return trace_msg_append_costr(msg, "division operator");
        case TT_BINOP_MOD:      return trace_msg_append_costr(msg, "modulus operator");
        case TT_BINOP_PLUS:     return trace_msg_append_costr(msg, "binary plus-operator");
        case TT_BINOP_MINUS:    return trace_msg_append_costr(msg, "binary minus-operator");
        case TT_KW_IF:
        case TT_KW_ELSE:
        case TT_KW_FOR:
        case TT_KW_RETURN:
        case TT_KW_FUN_DEF:     return trace_msg_append_costr(msg, "keyword");
        case TT_CMP_EQ:
        case TT_CMP_NEQ:
        case TT_CMP_GT_EQ:
        case TT_CMP_LT_EQ:      return trace_msg_append_costr(msg, "comparison operator");
        case TT_IMPORT:         return trace_msg_append_costr(msg, "from host to script import");
        case TT_EXPORT:         return trace_msg_append_costr(msg, "from script to host export");
        default:                return trace_msg_append_costr(msg, "unknown");
    }
}

inline static void trace_not_implemented(trace_t* trace, char* location) {
    trace_msg_t* m = trace_create_message(trace, TM_INTERNAL_ERROR, trace_no_ref());
    trace_msg_append(m, location, strnlen(location, (TRACE_MSG_MAX_LEN-29)));
    trace_msg_append_costr(m, " --- NOT IMPLEMENTED (YET).");
}

inline static int trace_msg_append_srcref(trace_msg_t* msg, srcref_t ref) {
    return trace_msg_append(msg,
        srcref_ptr(ref),
        srcref_len(ref));
}

inline static int trace_msg_append_sstr(trace_msg_t* msg, sstr_t* sstr) {
    return trace_msg_append(msg,
        sstr_ptr(sstr),
        sstr_len(sstr));
}

inline static int trace_msg_append_ift_type(trace_msg_t* msg, ift_t type) {
    sstr_t s = ift_type_to_sstr(type);
    return trace_msg_append_sstr(msg, &s);
}

inline static int trace_sprint_location(cstr_t cstr, srcref_t ref, char* filepath) {

    if( ref.source == NULL ) {
        return 0;
    }

    if(ref.idx_start > ref.idx_end) {
        size_t tmp = ref.idx_end;
        ref.idx_end = ref.idx_start;
        ref.idx_start = tmp;
    }

    size_t line = 0;
    size_t column = 0;

    for (size_t i = 0; i < ref.idx_start; i++) {
        if(ref.source[i] == '\n') {
            line ++;
            column = 0;
        } else {
            column ++;
        }
    }

    line += 1;
    column += 1;

    if( filepath != NULL ) {
        return cstr_append_fmt(cstr, "%s:%d:%d: ",
            filepath,
            (uint32_t) line,
            (uint32_t) column);
    } else {
        return cstr_append_fmt(cstr, "<unknown file>:%d:%d: ",
            (uint32_t) line,
            (uint32_t) column);
    }
}

inline static int trace_sprint_prefix(cstr_t cstr, trace_msg_t* msg) {
    switch(msg->type) {
        case TM_NONE:           return 0;
        case TM_OUT_OF_MEMORY:  return cstr_append_fmt(cstr, "ERROR: OUT OF SYSTEM MEMORY ");
        case TM_INTERNAL_ERROR: return cstr_append_fmt(cstr, "ERROR (INTERNAL PANIC): ");
        case TM_WARNING:        return cstr_append_fmt(cstr, "WARNING: ");
        case TM_INFO:           return cstr_append_fmt(cstr, "INFO: ");
        default:                return cstr_append_fmt(cstr, "ERROR: ");
    }
}

inline static int trace_sprint_msg(cstr_t cstr, trace_msg_t* msg) {

    int pres = trace_sprint_prefix(cstr, msg);
    if( pres < 0 )
        return pres;

    pres = trace_sprint_location(cstr, msg->source_location, msg->source_path);
    if( pres < 0 )
        return pres;
    
    if( msg->length >= TRACE_MSG_MAX_LEN ) {
        return cstr_append_fmt(cstr, "%.*s\n", TRACE_MSG_MAX_LEN, msg->message);
    } else if (msg->length > 0) {
        return cstr_append_fmt(cstr, "%.*s\n", (int) msg->length, msg->message);
    }

    return 0;
}

inline static void trace_sprint(cstr_t cstr, trace_t* trace) {
    for (size_t i = 0; i < trace->message_count; i++) {
        trace_sprint_msg(cstr, &trace->messages[i]);
    }
}

inline static size_t trace_get_error_count(trace_t* trace) {
    return trace->error_count;
}

inline static size_t trace_get_message_count(trace_t* trace) {
    return trace->message_count;
}

inline static void trace_clear(trace_t* trace) {
    //memset(trace->messages, 0, sizeof(trace_msg_t) * trace->message_count);
    trace->error_count = 0;
    trace->message_count = 0;
}


#endif // TRACE_H_
