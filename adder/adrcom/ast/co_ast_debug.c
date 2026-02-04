#include "adrcom/ast/co_ast_debug.h"
#include "adrcom/ast/co_ast.h"
#include <stdarg.h>

typedef enum txt_style_t {

    TXT_GREY       = 0,
    TXT_RED        = 1,
    TXT_GREEN      = 2,
    TXT_YELLOW     = 3,
    TXT_BLUE       = 4,
    TXT_PURPLE     = 5,
    TXT_CYAN       = 6,
    TXT_WHITE      = 7,

    TXT_BOLD        = 0x0100,
    TXT_UNDERLINED  = 0x0400,
    TXT_HIGHLIGHED  = 0x1000,
    TXT_ENABLED     = 0x8000

} txt_style_t;

#define STYLE_ERROR    (TXT_ENABLED|TXT_RED)
#define STYLE_WARNING  (TXT_ENABLED|TXT_YELLOW)

#define STYLE_VALUE    (TXT_ENABLED|TXT_GREY|TXT_HIGHLIGHED)
#define STYLE_SYMBOL   (TXT_ENABLED|TXT_YELLOW)
#define STYLE_TYPE     (TXT_ENABLED|TXT_GREEN)
#define STYLE_UNKNOWN  (TXT_ENABLED|TXT_RED)
#define STYLE_KEYWORD  (TXT_ENABLED|TXT_PURPLE)
#define STYLE_VARIABLE (TXT_ENABLED|TXT_CYAN|TXT_HIGHLIGHED)

size_t _ast_to_code(cstr_t* s, ast_t* n, int l);

size_t _check(int ret) {
    if(ret < 0) {
        sh_log_warning("ast debug check failed");
        return 0;
    }
    return (size_t) ret;
}

size_t _append_style_begin(cstr_t* str, txt_style_t ts) {

#if AST_DEBUG_ENABLE_ANSI_COLOR > 0

    if((ts & TXT_ENABLED) == 0)
        return 0;

    // "\033["STYLE";"COLOR"m"TEXT"\033[0m"

    int base = (ts & TXT_HIGHLIGHED) > 0 ? 90 : 30;
    int style = (ts >> 8) & 0x0F;

    size_t len = 0;
    len += _check(cstr_append(str, "\033["));
    len += _check(cstr_append_fmt(str, "%d", style));
    len += _check(cstr_append(str, ";"));
    len += _check(cstr_append_fmt(str, "%dm", base + (int)(ts & 0xFF)));

    return len;
#else

    (void) str;
    (void) ts;
    return 0;

#endif
}

size_t _append_style_end(cstr_t* str, txt_style_t ts) {
#if AST_DEBUG_ENABLE_ANSI_COLOR > 0
    if((ts & TXT_ENABLED) == 0)
        return 0;
    return _check(cstr_append(str, "\033[0m"));
#else
    (void) str;
    return 0;
#endif
}

size_t append_styled_fmt(cstr_t* str, txt_style_t style, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t res = 0;
    res += _append_style_begin(str, style);
    res += _check(cstr_vappend_fmt(str, fmt, args));
    res += _append_style_end(str, style);
    va_end(args);
    return res;
}

size_t append_styled_const(cstr_t* str, txt_style_t style, const char* text) {
    size_t res = 0;
    res += _append_style_begin(str, style);
    res += _check(cstr_append(str, text));
    res += _append_style_end(str, style);
    return res;
}

size_t append_styled_len(cstr_t* str, txt_style_t style, char* text, int len) {
    size_t res = 0;
    res += _append_style_begin(str, style);
    res += _check(cstr_append_len(str, text, len));
    res += _append_style_end(str, style);
    return res;
}

size_t append_code_unaop(cstr_t* s, char* opsym, ast_t* n) {
    size_t len = append_styled_const(s, STYLE_KEYWORD, opsym);
    len += _ast_to_code(s, n->as.items[AST_UNAOP_INNER], 0);
    return len;
}

size_t append_code_binop(cstr_t* s, char* opsym, ast_t* n) {
    size_t len = _ast_to_code(s, n->as.items[AST_BINOP_LEFT], 0);
    len += append_styled_fmt(s, STYLE_KEYWORD, " %s ", opsym);
    return len + _ast_to_code(s, n->as.items[AST_BINOP_RIGHT], 0);
}

size_t append_code_items(cstr_t* s, char* sep, ast_t* n) {
    size_t len = 0;
    for(size_t i = 0; i < (size_t) n->size; i++) {
        len += _ast_to_code(s, n->as.items[i], 0);
        if( (int) i < (n->size - 1) && sep != NULL )
            len += append_styled_const(s, 0, sep);
    }
    return len;
}

size_t append_indent(cstr_t* s, int indent) {
    if(indent <= 0)
        return 0;
    indent = indent * AST_DEBUG_CODE_INDENT_SPACES;
    char buf[indent+1];
    memset(buf, ' ', indent);
    buf[indent] = '\0';
    return append_styled_const(s, 0, buf);
}

bool is_statement(ast_t* n) {
    return n->tag != AST_IFCHAIN
        && n->tag != AST_FOREACH
        && n->tag != AST_FUNDEFN;
}

size_t append_diagnostics(cstr_t* s, ast_diags_t* diags, int l) {
    if(diags == NULL)
        return 0;
    size_t len = 0;
    for (int i = 0; i < diags->size; i++) {
        mk_cstr(dimsg, diag_to_string(diags->list[i], NULL));
        diag_to_string(diags->list[i], &dimsg);
        len += append_indent(s, l);
        if(diags->list[i]->kind == DIAG_ERROR) {
            len += append_styled_const(s, STYLE_ERROR, "\u2620 ");
            len += append_styled_len(s, 0, dimsg.ptr, dimsg.maxlen);
        } else {
            len += append_styled_const(s, STYLE_WARNING, "\u26A0 ");
            len += append_styled_len(s, 0, dimsg.ptr, dimsg.maxlen);
        }
        len += append_styled_const(s, 0, "\n");
    }
    return len;
}

size_t collect_append_diagnostics(cstr_t* s, ast_t* n, int l) {
    size_t len = append_diagnostics(s, n->diagnostics, l);
    for(int i = 0; i < n->size; i++) {
        len += collect_append_diagnostics(s, n->as.items[i], l);
    }
    return len;
}

size_t _ast_to_code(cstr_t* s, ast_t* n, int l) {
    switch(n->tag) {
        case AST_UNDEFINED:         return append_styled_const(s, STYLE_UNKNOWN, "!undefined!");
        case AST__BEGIN_VALUES:     return append_styled_const(s, STYLE_UNKNOWN, "!begin values!");
        case AST_INT:               return append_styled_fmt(s, STYLE_VALUE, "%d", n->as.value_int);
        case AST_FLOAT:             return append_styled_fmt(s, STYLE_VALUE, "%.8g", n->as.value_float);
        case AST_BOOL:              return append_styled_fmt(s, STYLE_VALUE, "%s", n->as.value_bool ? "true" : "false");
        case AST_CHAR:              return append_styled_fmt(s, STYLE_VALUE, "%.*s", 1, &n->as.value_char);
        case AST_STRING:            return append_styled_fmt(s, STYLE_VALUE, "\"%.*s\"", (int) srcref_len(n->as.srcref), srcref_ptr(n->as.srcref));
        case AST_SYMBOL:            return append_styled_fmt(s, STYLE_SYMBOL, "%.*s", (int) srcref_len(n->as.srcref), srcref_ptr(n->as.srcref));
        case AST_ARRAY: {
            size_t len = append_styled_const(s, 0, "[");
            len += append_code_items(s, ", ", n);
            len += append_styled_const(s, 0, "]");
            return len;
        };
        case AST__END_VALUES: return append_styled_const(s, STYLE_UNKNOWN, "!end values!");
        case AST__BEGIN_UNARY_OPERATORS: return append_styled_const(s, STYLE_UNKNOWN, "!begin unary operators!");
        case AST_UNA_NOT: return append_code_unaop(s, "not ", n);
        case AST_UNA_NEG: return append_code_unaop(s, "-", n);
        case AST__END_UNARY_OPERATORS: return append_styled_const(s, STYLE_UNKNOWN, "!end unary operators!");
        case AST__BEGIN_BINARY_OPERATORS: return append_styled_const(s, STYLE_UNKNOWN, "!begin binary operators!");
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
        case AST__END_BINARY_OPERATORS: return append_styled_const(s, STYLE_UNKNOWN, "!end binary operators!");
        case AST__BEGIN_HIGH_LEVEL:     return append_styled_const(s, STYLE_UNKNOWN, "!begin highlevel!");
        case AST_ARGLIST:   return append_code_items(s, ", ", n);
        case AST_VARREF: {
            srcref_t srcref = n->as.items[AST_VARREF_SYMBOL]->as.srcref;
            return append_styled_fmt(s, STYLE_VARIABLE, "%.*s",
                (int) srcref_len(srcref),
                srcref_ptr(srcref));
        };
        case AST_BLOCK: {
            size_t len = 0;
            len += append_styled_const(s, 0, " {\n");
            for(size_t i = 0; i < (size_t) n->size; i++) {
                len += append_indent(s, l + 1);
                len += _ast_to_code(s, n->as.items[i], l + 1);
                bool stmt = is_statement(n->as.items[i]);
                len += append_styled_const(s, 0, stmt ? ";\n" : "\n");
                len += collect_append_diagnostics(s, n->as.items[i], l + 1);
            }
            len += append_indent(s, l);
            len += append_styled_const(s, 0, "}");
            return len;
        };
        case AST_TYDESCR: {
            srcref_t srcref = n->as.items[AST_TYDESCR_SYMBOL]->as.srcref;
            size_t len = append_styled_fmt(s, STYLE_TYPE, "%.*s",
                (int) srcref_len(srcref),
                srcref_ptr(srcref));
            if( n->as.items[AST_TYDESCR_ARGLIST] == NULL )
                return len;
            if( n->as.items[AST_TYDESCR_ARGLIST]->size == 0 )
                return len;
            len += append_styled_const(s, 0, "<");  
            len += _ast_to_code(s, n->as.items[AST_TYDESCR_ARGLIST], l);
            len += append_styled_const(s, 0, ">");
            return len;
        };
        case AST_VARDECL: {
            size_t len = _ast_to_code(s, n->as.items[AST_VARDECL_TYDESCR], l);
            len += append_styled_const(s, 0, " ");  
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
                len += append_styled_fmt(s, STYLE_KEYWORD, "%s", "export ");
            else if ( ffi == AST_FUNSIGN_FFI_VAL_IMPORT )
                len += append_styled_fmt(s, STYLE_KEYWORD, "%s", "import ");
            len += _ast_to_code(s, n->as.items[AST_FUNSIGN_TYDESCR], l);
            len += append_styled_const(s, 0, " ");  
            len += _ast_to_code(s, n->as.items[AST_FUNSIGN_SYMBOL], l);
            len += append_styled_const(s, 0, "(");  
            len += _ast_to_code(s, n->as.items[AST_FUNSIGN_ARGLIST], l);
            len += append_styled_const(s, 0, ")");  
            return len;
        };
        case AST_FUNCALL: {
            size_t len = _ast_to_code(s, n->as.items[AST_FUNCALL_SYMBOL], l);
            len += append_styled_const(s, 0, "(");  
            len += _ast_to_code(s, n->as.items[AST_FUNCALL_ARGLIST], l);
            len += append_styled_const(s, 0, ")");
            return len;
        };
        case AST_FOREACH: {
            size_t len = append_styled_const(s, STYLE_KEYWORD, "for");
            len += append_styled_const(s, 0, " (");
            len += _ast_to_code(s, n->as.items[AST_FOREACH_VAR], l);
            len += append_styled_const(s, STYLE_KEYWORD, " in ");
            len += _ast_to_code(s, n->as.items[AST_FOREACH_COLL], l);
            len += append_styled_const(s, 0, ")");
            len += _ast_to_code(s, n->as.items[AST_FOREACH_BODY], l);
            return len;
        };
        case AST_IFCHAIN: {
            size_t len = 0;
            ast_t* next = n;

            do {

                len += append_styled_const(s, STYLE_KEYWORD, (n == next) ? "if" : " else if");
                len += append_styled_const(s, 0, " (");
                len += _ast_to_code(s, next->as.items[AST_IFCHAIN_COND], l);
                len += append_styled_const(s, 0, ")");
                len += _ast_to_code(s, next->as.items[AST_IFCHAIN_IFTRUE], l);
                next = next->as.items[AST_IFCHAIN_IFNEXT];

            } while( next != NULL && next->tag == AST_IFCHAIN );

            if( next != NULL && next->size > 0 ) {
                len += append_styled_const(s, STYLE_KEYWORD, " else ");
                len += _ast_to_code(s, next, l);
            }
            return len;
        };
        case AST_RETURN: {
            size_t len = 0;
            len += append_styled_const(s, STYLE_KEYWORD, "return ");
            len += _ast_to_code(s, n->as.items[AST_RETURN_EXPR], l);
            return len;
        };
        case AST_ASSIGN: {
            size_t len = 0;
            len += _ast_to_code(s, n->as.items[AST_ASSIGN_LEFT], l);
            len += append_styled_const(s, 0, " = ");
            len += _ast_to_code(s, n->as.items[AST_ASSIGN_RIGHT], l);
            return len;
        };
        case AST__END_HIGH_LEVEL: return append_styled_const(s, STYLE_UNKNOWN, "!end highlevel!");
        case AST__COUNT: return append_styled_const(s, STYLE_UNKNOWN, "!node type count!");
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

    txt_style_t style = 0;

    if( n->tag == AST_ARRAY )
        style = TXT_ENABLED | TXT_YELLOW;
    else if( ast_is_value(n) )
        style = TXT_ENABLED | TXT_YELLOW | TXT_HIGHLIGHED;
    else if( ast_is_binop(n) || ast_is_unop(n) )
        style = TXT_ENABLED | TXT_PURPLE | TXT_BOLD | TXT_HIGHLIGHED;
    else if( n->tag == AST_ARGLIST || n->tag == AST_BLOCK ) {
        style = TXT_ENABLED | TXT_BLUE;
        if( n->size > 0)
            style = style | TXT_BOLD;
    } else
        style = TXT_ENABLED | TXT_BLUE | TXT_HIGHLIGHED;

    len += append_styled_fmt(s, style, "(%s", tagstr + 4);

    switch(n->tag) {
        case AST_INT:    return len + append_styled_fmt(s, 0, " %d)", n->as.value_int);
        case AST_FLOAT:  return len + append_styled_fmt(s, 0, " %.8g)", n->as.value_float);
        case AST_BOOL:   return len + append_styled_fmt(s, 0, " %s)", n->as.value_bool ? "true" : "false");
        case AST_CHAR:   return len + append_styled_fmt(s, 0, " '%.*s)", 1, &n->as.value_char);
        case AST_STRING: return len + append_styled_fmt(s, 0, " \"%.*s\")", (int) srcref_len(n->as.srcref), srcref_ptr(n->as.srcref));
        case AST_SYMBOL: return len + append_styled_fmt(s, 0, "%.*s)", (int) srcref_len(n->as.srcref), srcref_ptr(n->as.srcref));
        default: {

            bool oneline = is_simple_node(n);
            for(int i = 0; i < n->size; i++) {

                if(oneline) {
                    len += append_styled_const(s, 0, " ");
                } else {
                    len += append_styled_const(s, 0, "\n");
                    len += append_indent(s, l + 2);
                }
                len += _ast_to_sexpr(s, n->as.items[i], l + 2);
            }
            len += append_styled_const(s, 0, ")");

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
            len += append_styled_const(s, 0, stmt ? ";\n" : "\n");
        }
        return len;
    } else {
        return _ast_to_code(s, n, 0);
    }

}

