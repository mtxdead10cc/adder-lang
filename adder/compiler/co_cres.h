#ifndef GVM_CRES_H_
#define GVM_CRES_H_

#include "sh_utils.h"
#include "co_types.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

inline static int cres_msg_add(cres_t* res, char* str, size_t slen) {
    int remaining = (int) CRES_MAX_MSG_LEN - (int) res->msg_len;
    if( remaining <= 0 || slen == 0 )
        return 0;
    int write_len = clamp((int)slen, 0, remaining);
    memcpy(res->msg + res->msg_len, str, write_len * sizeof(char));
    res->msg_len += write_len;
    return write_len;
}

#define cres_msg_add_costr(RES, STR) cres_msg_add((RES), (STR), (sizeof((STR))/sizeof(char)) - 1)

inline static int cres_msg_add_token_type_name(cres_t* res, token_type_t type) {
    switch (type) {
        case TT_INITIAL:        return cres_msg_add_costr(res, "initial token");
        case TT_SPACE:          return cres_msg_add_costr(res, "space");
        case TT_COMMENT:        return cres_msg_add_costr(res, "comment");
        case TT_SYMBOL:         return cres_msg_add_costr(res, "symbol");
        case TT_NUMBER:         return cres_msg_add_costr(res, "number");
        case TT_BOOLEAN:        return cres_msg_add_costr(res, "bool");
        case TT_STRING:         return cres_msg_add_costr(res, "string");
        case TT_SEPARATOR:      return cres_msg_add_costr(res, "separator");
        case TT_STATEMENT_END:  return cres_msg_add_costr(res, "end of statement");
        case TT_ARROW:          return cres_msg_add_costr(res, "function return type arrow");
        case TT_ASSIGN:         return cres_msg_add_costr(res, "assignment");
        case TT_CMP_LT:         return cres_msg_add_costr(res, "opening angle bracket alt. less than");
        case TT_CMP_GT:         return cres_msg_add_costr(res, "closing angle bracket alt. greater than");
        case TT_OPEN_PAREN:     return cres_msg_add_costr(res, "left parenthesis");
        case TT_CLOSE_PAREN:    return cres_msg_add_costr(res, "right parenthesis");
        case TT_OPEN_CURLY:     return cres_msg_add_costr(res, "opening brace");
        case TT_CLOSE_CURLY:    return cres_msg_add_costr(res, "closing brace");
        case TT_OPEN_SBRACKET:  return cres_msg_add_costr(res, "opening square bracket");
        case TT_CLOSE_SBRACKET: return cres_msg_add_costr(res, "closing square bracket");
        case TT_BINOP_AND:      return cres_msg_add_costr(res, "binary and-operator");
        case TT_BINOP_OR:       return cres_msg_add_costr(res, "binary or-operator");
        case TT_UNOP_NOT:       return cres_msg_add_costr(res, "unary not-operator");
        case TT_FINAL:          return cres_msg_add_costr(res, "final token");
        case TT_NOTHING:        return cres_msg_add_costr(res, "nothing");
        case TT_BINOP_MUL:      return cres_msg_add_costr(res, "multiply operator");
        case TT_BINOP_DIV:      return cres_msg_add_costr(res, "division operator");
        case TT_BINOP_MOD:      return cres_msg_add_costr(res, "modulus operator");
        case TT_BINOP_PLUS:     return cres_msg_add_costr(res, "binary plus-operator");
        case TT_BINOP_MINUS:    return cres_msg_add_costr(res, "binary minus-operator");
        case TT_KW_IF:
        case TT_KW_ELSE:
        case TT_KW_FOR:
        case TT_KW_RETURN:
        case TT_KW_FUN_DEF:     return cres_msg_add_costr(res, "keyword");
        case TT_CMP_EQ:
        case TT_CMP_NEQ:
        case TT_CMP_GT_EQ:
        case TT_CMP_LT_EQ:      return cres_msg_add_costr(res, "comparison operator");
        default:                return cres_msg_add_costr(res, "unknown");
    }
}

inline static int cres_msg_add_token(cres_t* res, token_t token) {
    cres_msg_add_token_type_name(res, token.type);
    cres_msg_add_costr(res, " ('");
    cres_msg_add(res,
        srcref_ptr(token.ref),
        srcref_len(token.ref));
    return cres_msg_add_costr(res, "')");
}

inline static int cres_msg_add_srcref(cres_t* res, srcref_t ref) {
    return cres_msg_add(res,
        srcref_ptr(ref),
        srcref_len(ref));
}

inline static int cres_fprint_location(FILE* stream, srcref_t ref, char* filepath) {

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
        return fprintf(stream, "%s:%d:%d: ",
            filepath,
            (uint32_t) line,
            (uint32_t) column);
    } else {
        return fprintf(stream, "<unknown file>:%d:%d: ",
            (uint32_t) line,
            (uint32_t) column);
    }
}

inline static int cres_fprint_prefix(FILE* stream, cres_t* res) {
    switch(res->code) {
        case R_OK:                  return 0;
        case R_ERR_OUT_OF_MEMORY:   return fprintf(stream, "[ERROR]: OUT OF SYSTEM MEMORY");
        case R_ERR_INTERNAL:        return fprintf(stream, "[ERROR]: INTERNAL PANIC");
        case R_ERR_STATEMENT:       return fprintf(stream, "[ERROR]: INVALID STATEMENT");
        default:                    return fprintf(stream, "[ERROR] ");
    }
}

inline static int cres_fprint(FILE* stream, cres_t* res, char* filepath_field) {

    int pres = cres_fprint_prefix(stream, res);
    if( pres < 0 )
        return pres;

    if( filepath_field != NULL ) {
        pres = cres_fprint_location(stream, res->ref, filepath_field);
        if( pres < 0 )
            return pres;
    }

    if( res->msg_len >= CRES_MAX_MSG_LEN ) {
        return fprintf(stream, "%.*s\n", CRES_MAX_MSG_LEN, res->msg);
    } else if (res->msg_len > 0) {
        return fprintf(stream, "%.*s\n", (int) res->msg_len, res->msg);
    }

    return 0;
}

inline static bool cres_is_ok(cres_t* res) {
    return res->code == R_OK;
}


inline static bool cres_has_error(cres_t* res) {
    return res->code != R_OK;
}

inline static bool cres_set_error(cres_t* res, cres_code_t code) {
    if( cres_has_error(res) ) {
        return false;
    }
    if( res->ref.source != NULL ) {
        printf( "internal-warning: previous source reference "
                "already present when writing error\n" );
    }
    res->code = code;
    return true;
}

inline static void cres_set_src_location(cres_t* res, srcref_t ref) {
    res->ref = ref;
}

#endif // GVM_CRES_H_