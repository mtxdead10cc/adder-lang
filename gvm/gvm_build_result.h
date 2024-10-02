#ifndef GVM_RESULT_H_
#define GVM_RESULT_H_

#include "gvm_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define RETURN_IF_ERROR(RES) do { if((RES).code!=R_OK) { return (RES); } } while (false)

inline static bool r_is_error(build_result_t res) {
    return res.code != R_OK;
}

inline static build_result_t r_ok(void) {
    return (build_result_t) {
        .code = R_OK,
        .args = { 0 },
        .location = { 0 }
    };
}

inline static build_result_t r_out_of_memory(void) {
    return (build_result_t) {
        .code = R_ER_OUT_OF_MEMORY,
        .args = { 0 },
        .location = { 0 }
    };
}

inline static build_result_t r_unrecognized_char(srcref_location_t location, char c) {
    build_result_t res = (build_result_t) { 0 };
    res.code = R_ER_UNRECOGNIZED_CHAR;
    res.args[0] = c,
    res.args[1] = 0,
    res.location = location;
    return res;
}

inline static build_result_t r_unexpected_token(srcref_location_t location, token_type_t expected, token_type_t actual) {
    return (build_result_t) {
        .code = R_ER_UNEXPECTED_TOKEN,
        .args = { actual, expected },
        .location = location
    };
}

inline static build_result_t r_invalid_format(srcref_location_t location) {
    return (build_result_t) {
        .code = R_ER_UNEXPECTED_TOKEN,
        .args = { 0 },
        .location = location
    };
}

inline static char* r_get_readable_tt(token_type_t type) {
    switch (type) {
        case TT_INITIAL: return "initial data (anything)";
        case TT_SPACE: return "space";
        case TT_COMMENT: return "comment";
        case TT_SYMBOL: return "symbol";
        case TT_NUMBER: return "number";
        case TT_BOOLEAN: return "bool";
        case TT_STRING: return "string";
        case TT_SEPARATOR: return "separator (,)";
        case TT_STATEMENT_END: return "end of statement (;)";
        case TT_ARROW: return "type arrow (->)";
        case TT_ASSIGN: return "assignment (=)";
        case TT_KW_IF: return "keyword (if)";
        case TT_KW_ELSE: return "keyword (else)";
        case TT_KW_FOR: return "keyword (for)";
        case TT_KW_RETURN: return "keyword (return)";
        case TT_KW_FUN_DEF: return "keyword (fun)";
        case TT_KW_AND: return "keyword (and)";
        case TT_KW_OR: return "keyword (or)";
        case TT_KW_NOT: return "keyword (not)";
        case TT_CMP_EQ: return "equal (==)";
        case TT_CMP_GT_EQ: return "greater than or equal (>=)";
        case TT_CMP_LT_EQ: return "less than or equal (<=)";
        case TT_LT_OR_OPEN_ABRACKET: return "<";
        case TT_GT_OR_CLOSE_ABRACKET: return ">";
        case TT_OPEN_PAREN: return "(";
        case TT_CLOSE_PAREN: return ")";
        case TT_OPEN_CURLY: return "{";
        case TT_CLOSE_CURLY: return "}";
        case TT_OPEN_SBRACKET: return "[";
        case TT_CLOSE_SBRACKET: return "}";
        case TT_FINAL: return "final token (end of text)";
        default: return "unknown";
    }
}

inline static void r_report_location(FILE* stream, srcref_location_t location) {
    if( location.filepath != NULL ) {
        fprintf(stream, "%s:%d:%d: ",
            location.filepath,
            (int) location.line,
            (int) location.column);
    }
}

inline static bool _print_full_token(token_type_t type) {
    return type == TT_BOOLEAN
        || type == TT_STRING
        || type == TT_SYMBOL
        || type == TT_NUMBER
        || type == TT_COMMENT;
}

inline static void r_report_error(FILE* stream, build_result_t res) {
    switch(res.code) {
        case R_OK: {
            fprintf(stream, "[ERROR] ");
            r_report_location(stream, res.location);
            fprintf(stream, "report-error called indicating no error\n");
        } break;
        case R_ER_OUT_OF_MEMORY: {
            fprintf(stream, "[ERROR] ");
            r_report_location(stream, res.location);
            fprintf(stream, "OUT OF MEMORY\n");
        } break;
        case R_ER_INVALID_STATE: {
            fprintf(stream, "[ERROR] ");
            r_report_location(stream, res.location);
            fprintf(stream, "(INTERNAL ERROR) INVALID STATE\n");
        } break;
        case R_ER_UNRECOGNIZED_CHAR: {
            fprintf(stream, "[ERROR] ");
            r_report_location(stream, res.location);
            fprintf(stream, "unrecognized character '%c'.\n",
                (char) res.args[0]);
        } break;
        case R_ER_INVALID_TOKEN_FORMAT: {
            fprintf(stream, "[ERROR] ");
            r_report_location(stream, res.location);
            fprintf(stream, "unexpected token format");
            fprintf(stream, " '%.*s'.",
                    (int) srcref_len(res.location.ref),
                    srcref_ptr(res.location.ref));
        } break;
        case R_ER_UNEXPECTED_TOKEN: {
            fprintf(stream, "[ERROR] ");
            r_report_location(stream, res.location);
            fprintf(stream, "unexpected token. expected ");
            token_type_t expected = res.args[1];
            size_t mask = 1;
            size_t count = 0;
            while ( mask < TT_FINAL ) {
                if( (mask & expected) > 0 ) {
                    if( count > 0 ) {
                        fprintf(stream, " or '%s'",
                            r_get_readable_tt(mask));
                    } else {
                        fprintf(stream, "'%s'",
                            r_get_readable_tt(mask));
                    }
                    count ++;
                }
                mask = (mask << 1);
            }
            fprintf(stream, " but found '%s",
                r_get_readable_tt(res.args[0]));
            if( _print_full_token(res.args[0]) ) {
                fprintf(stream, " (%.*s)",
                    (int) srcref_len(res.location.ref),
                    srcref_ptr(res.location.ref));
            }
            fprintf(stream, "'.\n");
        } break;
    }
}


#endif // GVM_RESULT_H_