#ifndef GVM_RESULT_H_
#define GVM_RESULT_H_

#include "gvm_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>


#define RETURN_IF_ERROR(RES) do { if((RES).code!=R_OK) { return (RES); } } while (true);                                         

inline static build_result_t res_ok(void) {
    return (build_result_t) {
        .code = R_OK,
        .info.nothing.ignore = 0,
        .ref = { 0 }
    };
}

inline static build_result_t res_err_out_of_host_memory(void) {
    return (build_result_t) {
        .code = R_ER_HOST_OUT_OF_MEMORY,
        .info.nothing.ignore = 0,
        .ref = { 0 }
    };
}

inline static build_result_t res_err_unexpected_token(srcref_t location, token_type_t expected, token_type_t actual) {
    return (build_result_t) {
        .code = R_ER_UNEXPECTED_TOKEN,
        .info.unexp_token = {
            .token_actual = actual,
            .token_expected_mask = expected
        },
        .ref = location
    };
}

inline static char* get_readable_token_type(token_type_t type) {
    switch (type) {
        case TT_INITIAL: return "anything";
        case TT_SPACE: return "space";
        case TT_COMMENT: return "comment";
        case TT_SYMBOL: return "symbol";
        case TT_NUMBER: return "number";
        case TT_BOOLEAN: return "boolean";
        case TT_SEPARATOR: return ",";
        case TT_STATEMENT_END: return ";";
        case TT_STRING: return "string";
        case TT_ARROW: return "->";
        case TT_ASSIGN: return "=";
        case TT_KW_IF: return "if";
        case TT_KW_ELSE: return "else";
        case TT_KW_FOR: return "for";
        case TT_KW_RETURN: return "return";
        case TT_KW_FUN_DEF: return "fun";
        case TT_KW_AND: return "and";
        case TT_KW_OR: return "or";
        case TT_KW_NOT: return "not";
        case TT_CMP_EQ: return "==";
        case TT_CMP_GT_EQ: return ">=";
        case TT_CMP_LT_EQ: return "<=";
        case TT_LT_OR_OPEN_ABRACKET: return "<";
        case TT_GT_OR_CLOSE_ABRACKET: return ">";
        case TT_OPEN_PAREN: return "(";
        case TT_CLOSE_PAREN: return ")";
        case TT_OPEN_CURLY: return "{";
        case TT_CLOSE_CURLY: return "}";
        case TT_OPEN_SBRACKET: return "[";
        case TT_CLOSE_SBRACKET: return "}";
        case TT_FINAL: return "end of text";
        default: return "unknown";
    }
}

inline static void res_report_location(FILE* stream, srcref_t ref) {
    srcref_location_t location = srcref_location_of(ref);
    if( ref.filepath != NULL ) {
        fprintf(stream, "%s:%d:%d: ",
            ref.filepath,
            (int) location.line,
            (int) location.column);
    }
}

inline static void res_report_error(FILE* stream, build_result_t res) {
    switch(res.code) {
        case R_OK: {
            fprintf(stream, "[ERROR] ");
            res_report_location(stream, res.ref);
            fprintf(stream, "report-error called indicating no error\n");
        } break;
        case R_ER_HOST_OUT_OF_MEMORY: {
            fprintf(stream, "[ERROR] ");
            res_report_location(stream, res.ref);
            fprintf(stream, "OUT OF MEMORY\n");
        } break;
        case R_ER_INVALID_STATE: {
            fprintf(stream, "[ERROR] ");
            res_report_location(stream, res.ref);
            fprintf(stream, "(INTERNAL ERROR) INVALID STATE\n");
        } break;
        case R_ER_UNEXPECTED_TOKEN: {
            fprintf(stream, "[ERROR] ");
            res_report_location(stream, res.ref);
            fprintf(stream, "unexpected token. expected ");
            token_type_t expected = res.info.unexp_token.token_expected_mask;
            size_t mask = 1;
            size_t count = 0;
            while ( mask < TT_FINAL ) {
                if( (mask & expected) > 0 ) {
                    if( count > 0 ) {
                        fprintf(stream, " or '%s'",
                            get_readable_token_type(mask));
                    } else {
                        fprintf(stream, "'%s'",
                            get_readable_token_type(mask));
                    }
                    count ++;
                }
                mask = (mask << 1);
            }
            fprintf(stream, " but found '%s'.\n",
                get_readable_token_type(res.info.unexp_token.token_actual));
        } break;
    }
}


#endif // GVM_RESULT_H_