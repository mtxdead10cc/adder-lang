#include "adrcom/ast/co_ast_debug.h"
#include "adrcom/ast/co_ast.h"
#include <stdarg.h>

#ifdef AST_DEBUG_ENABLE_ANSI_COLOR
# define STYLE(STYLE, COLOR, TEXT) "\033["STYLE";"COLOR"m"TEXT"\033[0m"
#else
# define STYLE(STYLE, COLOR, TEXT) TEXT
#endif

#define NORM "0" // normal
#define BOLD "1" // bold
#define UNDL "4" // underlined

#define GRAY        "30"
#define GRAY_HI     "90"
#define RED         "31"
#define RED_HI      "91"
#define GREEN       "32"
#define GREEN_HI    "92"
#define YELLOW      "33"
#define YELLOW_HI   "93"
#define BLUE        "34"
#define BLUE_HI     "94"
#define PURPLE      "35"
#define PURPLE_HI   "95"
#define CYAN        "36"
#define CYAN_HI     "96"
#define WHITE       "37"
#define WHITE_HI    "97"

#define STKEYWORD(S)  STYLE(NORM, PURPLE, S)
#define STUNKNOWN(S)  STYLE(BOLD, RED, S)
#define STVALUE(S)    STYLE(NORM, GRAY_HI, S)
#define STSYMBOL(S)   STYLE(NORM, YELLOW, S)
#define STTYPE(S)     STYLE(NORM, GREEN, S)
#define STVARIABLE(S) STYLE(NORM, CYAN_HI, S)

size_t _ast_to_code(cstr_t* s, ast_t* n, int l);

size_t append_to_cstr(cstr_t* s, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int res = cstr_vappend_fmt(s, fmt, args);
    va_end(args);
    if( res < 0 ) {
        sh_log_warning("failed to append (ast_to_string)");
        return 0;
    }
    return (size_t) res;
}

size_t append_code_unaop(cstr_t* s, char* opsym, ast_t* n) {
    size_t len = append_to_cstr(s, "%s", opsym);
    len += _ast_to_code(s, n->as.items[AST_UNAOP_INNER], 0);
    return len;
}

size_t append_code_binop(cstr_t* s, char* opsym, ast_t* n) {
    size_t len = _ast_to_code(s, n->as.items[AST_BINOP_LEFT], 0);
    len += append_to_cstr(s, " %s ", opsym);
    return len + _ast_to_code(s, n->as.items[AST_BINOP_RIGHT], 0);
}

size_t append_code_items(cstr_t* s, char* sep, ast_t* n) {
    size_t len = 0;
    for(size_t i = 0; i < (size_t) n->size; i++) {
        len += _ast_to_code(s, n->as.items[i], 0);
        if( (int) i < (n->size - 1) && sep != NULL )
            len += append_to_cstr(s, "%s", sep);
    }
    return len;
}

size_t append_indent(cstr_t* s, int indent) {
    if(indent <= 0)
        return 0;
    indent = indent * 2;
    char buf[indent+1];
    memset(buf, ' ', indent);
    buf[indent] = '\0';
    return append_to_cstr(s, "%s", buf);
}

bool is_statement(ast_t* n) {
    return n->tag != AST_IFCHAIN
        && n->tag != AST_FOREACH
        && n->tag != AST_FUNDEFN;
}

size_t _ast_to_code(cstr_t* s, ast_t* n, int l) {
    switch(n->tag) {
        case AST_UNDEFINED: return append_to_cstr(s, STUNKNOWN("!undefined!"));
        case AST__BEGIN_VALUES: return append_to_cstr(s, STUNKNOWN("!begin values!"));
        case AST_INT:  return append_to_cstr(s,   STVALUE("%d"), n->as.value_int);
        case AST_FLOAT: return append_to_cstr(s,  STVALUE("%.8g"), n->as.value_float);
        case AST_BOOL: return append_to_cstr(s,   STVALUE("%s"), n->as.value_bool ? "true" : "false");
        case AST_CHAR: return append_to_cstr(s,   STVALUE("%.*s"), 1, &n->as.value_char);
        case AST_STRING: return append_to_cstr(s, STVALUE("\"%.*s\""), (int) srcref_len(n->as.srcref), srcref_ptr(n->as.srcref));
        case AST_SYMBOL: return append_to_cstr(s, STSYMBOL("%.*s"), (int) srcref_len(n->as.srcref), srcref_ptr(n->as.srcref));
        case AST_ARRAY: {
            size_t len = append_to_cstr(s, "%s", "[");
            len += append_code_items(s, ", ", n);
            len += append_to_cstr(s, "%s", "]");
            return len;
        };
        case AST__END_VALUES: return append_to_cstr(s, STUNKNOWN("!end values!"));
        case AST__BEGIN_UNARY_OPERATORS: return append_to_cstr(s, STUNKNOWN("!begin unary operators!"));
        case AST_UNA_NOT: return append_code_unaop(s, STKEYWORD("not "), n);
        case AST_UNA_NEG: return append_code_unaop(s, STKEYWORD("-"), n);
        case AST__END_UNARY_OPERATORS: return append_to_cstr(s, "!end unary operators!");
        case AST__BEGIN_BINARY_OPERATORS: return append_to_cstr(s, "!begin binary operators!");
        case AST_BIN_MUL:   return append_code_binop(s, "*", n);
        case AST_BIN_DIV:   return append_code_binop(s, "/", n);
        case AST_BIN_MOD:   return append_code_binop(s, "%", n);
        case AST_BIN_ADD:   return append_code_binop(s, "+", n);
        case AST_BIN_SUB:   return append_code_binop(s, "-", n);
        case AST_BIN_XOR:   return append_code_binop(s, "xor", n);
        case AST_BIN_LT:    return append_code_binop(s, "<", n);
        case AST_BIN_GT:    return append_code_binop(s, ">", n);
        case AST_BIN_LT_EQ: return append_code_binop(s, "<=", n);
        case AST_BIN_GT_EQ: return append_code_binop(s, ">=", n);
        case AST_BIN_EQ:    return append_code_binop(s, "==", n);
        case AST_BIN_NEQ:   return append_code_binop(s, "!=", n);
        case AST_BIN_OR:    return append_code_binop(s, "or", n);
        case AST_BIN_AND:   return append_code_binop(s, "and", n);
        case AST__END_BINARY_OPERATORS: return append_to_cstr(s, STUNKNOWN("!end binary operators!"));
        case AST__BEGIN_HIGH_LEVEL:     return append_to_cstr(s, STUNKNOWN("!begin highlevel!"));
        case AST_ARGLIST:   return append_code_items(s, ", ", n);
        case AST_VARREF: {
            srcref_t srcref = n->as.items[AST_VARREF_SYMBOL]->as.srcref;
            return append_to_cstr(s, STVARIABLE("%.*s"),
                (int) srcref_len(srcref),
                srcref_ptr(srcref));
        };
        case AST_BLOCK: {
            size_t len = 0;
            len += append_to_cstr(s, " {\n");
            for(size_t i = 0; i < (size_t) n->size; i++) {
                len += append_indent(s, l + 1);
                len += _ast_to_code(s, n->as.items[i], l + 1);
                bool stmt = is_statement(n->as.items[i]);
                len += append_to_cstr(s, "%s", stmt ? ";\n" : "\n");
            }
            len += append_indent(s, l);
            len += append_to_cstr(s, "}");
            return len;
        };
        case AST_TYDESCR: {
            srcref_t srcref = n->as.items[AST_TYDESCR_SYMBOL]->as.srcref;
            size_t len = append_to_cstr(s, STTYPE("%.*s"),
                (int) srcref_len(srcref),
                srcref_ptr(srcref));
            if( n->as.items[AST_TYDESCR_ARGLIST] == NULL )
                return len;
            if( n->as.items[AST_TYDESCR_ARGLIST]->size == 0 )
                return len;
            len += append_to_cstr(s, "%s", "<");  
            len += _ast_to_code(s, n->as.items[AST_TYDESCR_ARGLIST], l);
            len += append_to_cstr(s, "%s", ">");
            return len;
        };
        case AST_VARDECL: {
            size_t len = _ast_to_code(s, n->as.items[AST_VARDECL_TYDESCR], l);
            len += append_to_cstr(s, "%s", " ");  
            len += _ast_to_code(s, n->as.items[AST_VARDECL_VARREF], l);
            return len;
        };
        case AST_FUNDEFN: {
            size_t len = _ast_to_code(s, n->as.items[AST_FUNDEFN_FUNSIGN], l);
            len += _ast_to_code(s, n->as.items[AST_FUNDEFN_BODY], l);
            return len;
        };
        case AST_FUNSIGN: {
            size_t len = 0;
            int ffi = n->as.items[AST_FUNSIGN_FFI]->as.value_int;
            if( ffi == AST_FUNSIGN_FFI_VAL_EXPORT )
                len += append_to_cstr(s, STKEYWORD("%s"), "export ");
            else if ( ffi == AST_FUNSIGN_FFI_VAL_IMPORT )
                len += append_to_cstr(s, STKEYWORD("%s"), "import ");
            len += _ast_to_code(s, n->as.items[AST_FUNSIGN_TYDESCR], l);
            len += append_to_cstr(s, "%s", " ");  
            len += _ast_to_code(s, n->as.items[AST_FUNSIGN_SYMBOL], l);
            len += append_to_cstr(s, "%s", "(");  
            len += _ast_to_code(s, n->as.items[AST_FUNSIGN_ARGLIST], l);
            len += append_to_cstr(s, "%s", ")");  
            return len;
        };
        case AST_FUNCALL: {
            size_t len = _ast_to_code(s, n->as.items[AST_FUNCALL_SYMBOL], l);
            len += append_to_cstr(s, "%s", "(");  
            len += _ast_to_code(s, n->as.items[AST_FUNCALL_ARGLIST], l);
            len += append_to_cstr(s, "%s", ")");
            return len;
        };
        case AST_FOREACH: {
            size_t len = append_to_cstr(s, STKEYWORD("%s")"(", "for");
            len += _ast_to_code(s, n->as.items[AST_FOREACH_VAR], l);
            len += append_to_cstr(s, STKEYWORD(" %s "), "in");
            len += _ast_to_code(s, n->as.items[AST_FOREACH_COLL], l);
            len += append_to_cstr(s, "%s", ")");  
            len += _ast_to_code(s, n->as.items[AST_FOREACH_BODY], l);
            return len;
        };
        case AST_IFCHAIN: {
            size_t len = 0;
            ast_t* next = n;

            do {

                len += append_to_cstr(s, STKEYWORD("%s")" (", (n == next) ? "if" : " else if");
                len += _ast_to_code(s, next->as.items[AST_IFCHAIN_COND], l);
                len += append_to_cstr(s, "%s", ")");
                len += _ast_to_code(s, next->as.items[AST_IFCHAIN_IFTRUE], l);
                next = next->as.items[AST_IFCHAIN_IFNEXT];

            } while( next != NULL && next->tag == AST_IFCHAIN );

            if( next != NULL && next->size > 0 ) {
                len += append_to_cstr(s, STKEYWORD("%s"), " else ");
                len += _ast_to_code(s, next, l);
            }
            return len;
        };
        case AST_RETURN: {
            size_t len = 0;
            len += append_to_cstr(s, STKEYWORD("%s"), "return ");
            len += _ast_to_code(s, n->as.items[AST_RETURN_EXPR], l);
            return len;
        };
        case AST_ASSIGN: {
            size_t len = 0;
            len += _ast_to_code(s, n->as.items[AST_ASSIGN_LEFT], l);
            len += append_to_cstr(s, "%s", " = ");
            len += _ast_to_code(s, n->as.items[AST_ASSIGN_RIGHT], l);
            return len;
        };
        case AST__END_HIGH_LEVEL: return append_to_cstr(s, STUNKNOWN("!end highlevel!"));
        case AST__COUNT: return append_to_cstr(s, STUNKNOWN("!node type count!"));
        default: {
            return 0;
        };
    }
}

bool is_simple_node(ast_t* n) {
    for(int i = 0; i < n->size; i++) {
        if(ast_is_value(n->as.items[i]) == false)
            return false;
    }
    return true;
}

size_t _ast_to_sexpr(cstr_t* s, ast_t* n, int l) {

    size_t len = 0;

    const char* tagstr = ast_tag_to_string(n->tag);

    if( n->tag == AST_ARRAY )
        len += append_to_cstr(s, "("STYLE(BOLD, YELLOW, "%s"), tagstr + 4);
    else if( ast_is_value(n) )
        len += append_to_cstr(s, "("STYLE(NORM, YELLOW_HI, "%s"), tagstr + 4);
    else if( ast_is_binop(n) || ast_is_unop(n) )
        len += append_to_cstr(s, "("STYLE(BOLD, PURPLE, "%s"), tagstr + 8);
    else if( n->tag == AST_ARGLIST || n->tag == AST_BLOCK ) {
        if( n->size > 0)
            len += append_to_cstr(s, "("STYLE(BOLD, BLUE, "%s"), tagstr + 4);
        else 
            len += append_to_cstr(s, "("STYLE(NORM, BLUE, "%s"), tagstr + 4);
    } else
        len += append_to_cstr(s, "("STYLE(BOLD, BLUE_HI, "%s"), tagstr + 4);

    switch(n->tag) {
        case AST_INT:    return len + append_to_cstr(s, STYLE(NORM, WHITE, " %d")")", n->as.value_int);
        case AST_FLOAT:  return len + append_to_cstr(s, STYLE(NORM, WHITE, " %.8g")")", n->as.value_float);
        case AST_BOOL:   return len + append_to_cstr(s, STYLE(NORM, WHITE, " %s")")", n->as.value_bool ? "true" : "false");
        case AST_CHAR:   return len + append_to_cstr(s, STYLE(NORM, WHITE, " '%.*s'")")", 1, &n->as.value_char);
        case AST_STRING: return len + append_to_cstr(s, STYLE(NORM, WHITE, " \"%.*s\"")")", (int) srcref_len(n->as.srcref), srcref_ptr(n->as.srcref));
        case AST_SYMBOL: return len + append_to_cstr(s, " "STYLE(UNDL, WHITE, "%.*s")")", (int) srcref_len(n->as.srcref), srcref_ptr(n->as.srcref));
        default: {

            bool oneline = is_simple_node(n);
            for(int i = 0; i < n->size; i++) {

                if(oneline) {
                    len += append_to_cstr(s, "%s", " ");
                } else {
                    len += append_to_cstr(s, "%s", "\n");
                    len += append_indent(s, l + 2);
                }
                len += _ast_to_sexpr(s, n->as.items[i], l + 2);
            }
            len += append_to_cstr(s, "%s", ")");

            return len;
        };
    }
}

size_t ast_to_string(cstr_t* s, ast_t* n, ast_dbgstyle_t style) {

    if(style == AST_DBG_SEXPR)
        return _ast_to_sexpr(s, n, 0);

    if( n->tag == AST_BLOCK ) {
        size_t len = 0;
        for(size_t i = 0; i < (size_t) n->size; i++) {
            len += _ast_to_code(s, n->as.items[i], 0);
            bool stmt = is_statement(n->as.items[i]);
            len += append_to_cstr(s, "%s", stmt ? ";\n" : "\n");
        }
        return len;
    } else {
        return _ast_to_code(s, n, 0);
    }

}

