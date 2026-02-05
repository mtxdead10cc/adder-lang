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
}

size_t _append_style_end(cstr_t* str, txt_style_t ts) {
    if((ts & TXT_ENABLED) == 0)
        return 0;
    return _check(cstr_append(str, "\033[0m"));
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

txt_style_t _get_style(ast_t* n) {

    if(n->diagnostics != NULL) {
        if(n->diagnostics->kind == DIAG_ERROR)
            return (STYLE_ERROR | TXT_UNDERLINED);
        if(n->diagnostics->kind == DIAG_WARNING)
            return (STYLE_WARNING | TXT_UNDERLINED);
    }

    ast_tag_t tag = n->tag;
    if(tag == AST_SYMBOL)
        return STYLE_SYMBOL;
    else if(tag == AST_TYDESCR)
        return STYLE_TYPE;
    else if(tag == AST_VARREF)
        return STYLE_VARIABLE;
    else if(ast_tag_is_value(tag))
        return STYLE_VALUE;
    else if(ast_tag_is_binop(tag) || ast_tag_is_unop(tag))
        return 0;
    else if(ast_tag_is_highlevel(tag))
        return STYLE_KEYWORD;
    
    return STYLE_UNKNOWN;
}

size_t append_code_unaop(cstr_t* s, char* opsym, ast_t* n) {
    size_t len = append_styled_const(s, _get_style(n), opsym);
    len += _ast_to_code(s, n->as.items[AST_UNAOP_INNER], 0);
    return len;
}

size_t append_code_binop(cstr_t* s, char* opsym, ast_t* n) {
    size_t len = _ast_to_code(s, n->as.items[AST_BINOP_LEFT], 0);
    len += append_styled_fmt(s, _get_style(n), " %s ", opsym);
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

        txt_style_t border = TXT_ENABLED | TXT_GREEN;

        if(diags->list[i]->kind == DIAG_ERROR)
            border = STYLE_ERROR;
        else if(diags->list[i]->kind == DIAG_WARNING)
            border = STYLE_WARNING;

        mk_cstr(dimsg, diag_to_string(diags->list[i], NULL));
        diag_to_string(diags->list[i], &dimsg);

        len += append_indent(s, l);

        int start = 0;

        for(int curr = 1; curr < dimsg.maxlen; curr++) {
            if(dimsg.ptr[curr] == '\n') {
                len += append_styled_const(s, border, "| ");
                len += append_styled_len(s, 0,
                    dimsg.ptr + start,
                    curr - start + 1);
                len += append_indent(s, l);
                curr += 1;
                start = curr;
            }
        }
    
        if(start < dimsg.maxlen) {
            len += append_styled_const(s, border, "| ");
            len += append_styled_len(s, 0, dimsg.ptr + start, dimsg.maxlen - start);
            len += append_styled_const(s, 0, "\n");
        }
        
        len += append_indent(s, l);
        len += append_styled_const(s, border, "'¨¨¨¨¨¨¨¨¨\n");

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

    txt_style_t style = _get_style(n);
    size_t length = 0;

    switch(n->tag) {
        case AST_UNDEFINED:
            length = append_styled_const(s, style, "!undefined!");
            break;
        case AST__BEGIN_VALUES:
            length = append_styled_const(s, style, "!begin values!");
            break;
        case AST_INT:
            length = append_styled_fmt(s, style, "%d",
                n->as.value.as._int);
            break;
        case AST_FLOAT:
            length = append_styled_fmt(s, style, "%.8g",
                n->as.value.as._float);
            break;
        case AST_BOOL:
            length = append_styled_fmt(s, style, "%s",
                n->as.value.as._bool
                    ? "true"
                    : "false");
            break;
        case AST_CHAR:
            length = append_styled_fmt(s, style, "%.*s",
                1, &n->as.value.as._char);
            break;
        case AST_STRING:
            length = append_styled_fmt(s, style, "\"%.*s\"",
                (int) srcref_len(n->as.value.srcref),
                srcref_ptr(n->as.value.srcref));
            break;
        case AST_SYMBOL:
            length = append_styled_fmt(s, style, "%.*s",
                (int) srcref_len(n->as.value.srcref),
                srcref_ptr(n->as.value.srcref));
            break;
        case AST_ARRAY: {
            size_t len = append_styled_const(s, 0, "[");
            len += append_code_items(s, ", ", n);
            len += append_styled_const(s, 0, "]");
            length = len;
        } break;
        case AST__END_VALUES:
            length = append_styled_const(s, style, "!end values!");
            break;
        case AST__BEGIN_UNARY_OPERATORS:
            length = append_styled_const(s, style, "!begin unary operators!");
            break;
        case AST_UNA_NOT:
            length = append_code_unaop(s, "not ", n);
            break;
        case AST_UNA_NEG:
            length = append_code_unaop(s, "-", n);
            break;
        case AST__END_UNARY_OPERATORS:
            length = append_styled_const(s, style, "!end unary operators!");
            break;
        case AST__BEGIN_BINARY_OPERATORS:
            length = append_styled_const(s, style, "!begin binary operators!");
            break;
        case AST_BIN_MUL:
            length = append_code_binop(s, "*", n);
            break;
        case AST_BIN_DIV:
            length = append_code_binop(s, "/", n);
            break;
        case AST_BIN_MOD:
            length = append_code_binop(s, "%", n);
            break;
        case AST_BIN_ADD:
            length = append_code_binop(s, "+", n);
            break;
        case AST_BIN_SUB:
            length = append_code_binop(s, "-", n);
            break;
        case AST_BIN_XOR:
            length = append_code_binop(s, "xor", n);
            break;
        case AST_BIN_LT:
            length = append_code_binop(s, "<", n);
            break;
        case AST_BIN_GT:
            length = append_code_binop(s, ">", n);
            break;
        case AST_BIN_LT_EQ:
            length = append_code_binop(s, "<=", n);
            break;
        case AST_BIN_GT_EQ:
            length = append_code_binop(s, ">=", n);
            break;
        case AST_BIN_EQ:
            length = append_code_binop(s, "==", n);
            break;
        case AST_BIN_NEQ:
            length = append_code_binop(s, "!=", n);
            break;
        case AST_BIN_OR:
            length = append_code_binop(s, "or", n);
            break;
        case AST_BIN_AND:
            length = append_code_binop(s, "and", n);
            break;
        case AST__END_BINARY_OPERATORS:
            length = append_styled_const(s, style, "!end binary operators!"); break;
        case AST__BEGIN_HIGH_LEVEL:
            length = append_styled_const(s, style, "!begin highlevel!");
            break;
        case AST_ARGLIST:
            length = append_code_items(s, ", ", n);
            break;
        case AST_VARREF: {
            srcref_t srcref = n->as.items[AST_VARREF_SYMBOL]->as.value.srcref;
            length = append_styled_fmt(s, style, "%.*s",
                (int) srcref_len(srcref),
                srcref_ptr(srcref));
        } break;
        case AST_BLOCK: {
            size_t len = 0;
            for(size_t i = 0; i < (size_t) n->size; i++) {
                len += append_indent(s, l + 1);
                len += _ast_to_code(s, n->as.items[i], l + 1);
                bool stmt = is_statement(n->as.items[i]);
                len += append_styled_const(s, 0, stmt ? ";\n" : "\n");
                if(stmt) {
                    len += collect_append_diagnostics(s, n->as.items[i], l + 1);
                }
            }
            length = len;
        } break;
        case AST_TYDESCR: {
            srcref_t srcref = n->as.items[AST_TYDESCR_SYMBOL]->as.value.srcref;
            size_t len = append_styled_fmt(s, style, "%.*s",
                (int) srcref_len(srcref),
                srcref_ptr(srcref));
            if( n->as.items[AST_TYDESCR_ARGLIST] == NULL )
                length = len;
            if( n->as.items[AST_TYDESCR_ARGLIST]->size > 0 ) {
                len += append_styled_const(s, 0, "<");  
                len += _ast_to_code(s, n->as.items[AST_TYDESCR_ARGLIST], l);
                len += append_styled_const(s, 0, ">");
            }
            length = len;
        } break;
        case AST_VARDECL: {
            size_t len = _ast_to_code(s, n->as.items[AST_VARDECL_TYDESCR], l);
            len += append_styled_const(s, 0, " ");  
            len += _ast_to_code(s, n->as.items[AST_VARDECL_VARREF], l);
            length = len;
        } break;
        case AST_FUNDEFN: {
            // statement
            size_t len = _ast_to_code(s, n->as.items[AST_FUNDEFN_FUNSIGN], l);
            // error printing
            len += collect_append_diagnostics(s, n->as.items[AST_FUNDEFN_FUNSIGN], l + 1);
            len += append_diagnostics(s, n->diagnostics, l + 1);
            // body
            len += append_styled_const(s, 0, " {\n");
            len += _ast_to_code(s, n->as.items[AST_FUNDEFN_BODY], l);
            len += append_indent(s, l);
            len += append_styled_const(s, 0, "}");
            length = len;
        } break;
        case AST_FUNSIGN: {
            size_t len = 0;
            srcref_t ffi = n->as.items[AST_FUNSIGN_FFI]->as.value.srcref;
            if( srcref_equals_string(ffi, "export") )
                len += append_styled_fmt(s, style, "%s", "export ");
            else if ( srcref_equals_string(ffi, "import") )
                len += append_styled_fmt(s, style, "%s", "import ");
            len += _ast_to_code(s, n->as.items[AST_FUNSIGN_TYDESCR], l);
            len += append_styled_const(s, 0, " ");  
            len += _ast_to_code(s, n->as.items[AST_FUNSIGN_SYMBOL], l);
            len += append_styled_const(s, 0, "(");
            len += _ast_to_code(s, n->as.items[AST_FUNSIGN_ARGLIST], l);
            len += append_styled_const(s, 0, ")");
            length = len;
        } break;
        case AST_FUNCALL: {
            size_t len = _ast_to_code(s, n->as.items[AST_FUNCALL_SYMBOL], l);
            len += append_styled_const(s, 0, "(");  
            len += _ast_to_code(s, n->as.items[AST_FUNCALL_ARGLIST], l);
            len += append_styled_const(s, 0, ")");
            length = len;
        } break;
        case AST_FOREACH: {
            // statement
            size_t len = append_styled_const(s, style, "for");
            len += append_styled_const(s, 0, " (");
            len += _ast_to_code(s, n->as.items[AST_FOREACH_VAR], l);
            len += append_styled_const(s, style, " in ");
            len += _ast_to_code(s, n->as.items[AST_FOREACH_COLL], l);
            len += append_styled_const(s, 0, ") {\n");
            // error printing
            len += collect_append_diagnostics(s, n->as.items[AST_FOREACH_VAR], l + 1);
            len += collect_append_diagnostics(s, n->as.items[AST_FOREACH_COLL], l + 1);
            len += append_diagnostics(s, n->diagnostics, l +1);
            // body
            len += _ast_to_code(s, n->as.items[AST_FOREACH_BODY], l);
            len += append_indent(s, l);
            len += append_styled_const(s, 0, "}");
            length = len;
        } break;
        case AST_IFCHAIN: {

            size_t len = 0;
            ast_t* next = n;

            do {
                // statement
                len += append_styled_const(s, style, (n == next) ? "if" : " else if");
                len += append_styled_const(s, 0, " (");
                len += _ast_to_code(s, next->as.items[AST_IFCHAIN_COND], l);
                len += append_styled_const(s, 0, ") {\n");
                // error printing
                len += collect_append_diagnostics(s, next->as.items[AST_IFCHAIN_COND], l + 1);
                len += append_diagnostics(s, next->diagnostics, l +1);
                // body
                len += _ast_to_code(s, next->as.items[AST_IFCHAIN_IFTRUE], l);
                next = next->as.items[AST_IFCHAIN_IFNEXT];
                len += append_indent(s, l);
                len += append_styled_const(s, 0, "}");

            } while( next != NULL && next->tag == AST_IFCHAIN );

            if( next != NULL && next->size > 0 ) {
                // statement
                len += append_styled_const(s, style, " else {\n");
                // error printing
                len += append_diagnostics(s, next->diagnostics, l +1);
                // body
                len += _ast_to_code(s, next, l);
                len += append_indent(s, l);
                len += append_styled_const(s, 0, "}");
            }

            length = len;

        } break;
        case AST_RETURN: {
            size_t len = 0;
            len += append_styled_const(s, style, "return ");
            len += _ast_to_code(s, n->as.items[AST_RETURN_EXPR], l);
            length = len;
        } break;
        case AST_ASSIGN: {
            size_t len = 0;
            len += _ast_to_code(s, n->as.items[AST_ASSIGN_LEFT], l);
            len += append_styled_const(s, 0, " = ");
            len += _ast_to_code(s, n->as.items[AST_ASSIGN_RIGHT], l);
            length = len;
        } break;
        case AST__END_HIGH_LEVEL:
            length = append_styled_const(s, style, "!end highlevel!");
            break;
        case AST__COUNT:
            length = append_styled_const(s, style, "!node type count!");
            break;
        default:
            length = 0;
            break;
    }

    return length;
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
        case AST_INT:    return len + append_styled_fmt(s, 0, " %d)", n->as.value.as._int);
        case AST_FLOAT:  return len + append_styled_fmt(s, 0, " %.8g)", n->as.value.as._float);
        case AST_BOOL:   return len + append_styled_fmt(s, 0, " %s)", n->as.value.as._bool ? "true" : "false");
        case AST_CHAR:   return len + append_styled_fmt(s, 0, " '%.*s)", 1, &n->as.value.as._char);
        case AST_STRING: return len + append_styled_fmt(s, 0, " \"%.*s\")", (int) srcref_len(n->as.value.srcref), srcref_ptr(n->as.value.srcref));
        case AST_SYMBOL: return len + append_styled_fmt(s, 0, "%.*s)", (int) srcref_len(n->as.value.srcref), srcref_ptr(n->as.value.srcref));
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

