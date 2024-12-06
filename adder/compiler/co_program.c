#include "co_program.h"
#include "co_parser.h"
#include "co_compiler.h"
#include "co_trace.h"
#include "sh_asminfo.h"
#include "sh_types.h"
#include "sh_value.h"
#include <unistd.h>

char* unop_name(ast_unop_type_t type) {
    // TODO:
    // these should match the source code symbols
    // in the language itself
    switch(type) {
        case AST_UN_NEG: return "-";
        case AST_UN_NOT: return "not";
        default: return "";
    }
}

char* binop_name(ast_binop_type_t type) {
    // TODO:
    // these should match the source code symbols
    // in the language itself
    switch(type) {
        case AST_BIN_ADD:   return "+";
        case AST_BIN_SUB:   return "-";
        case AST_BIN_MUL:   return "*";
        case AST_BIN_DIV:   return "/";
        case AST_BIN_MOD:   return "%";
        case AST_BIN_AND:   return "and";
        case AST_BIN_OR:    return "or";
        case AST_BIN_XOR:   return "xor";
        case AST_BIN_EQ:    return "==";
        case AST_BIN_NEQ:   return "!=";
        case AST_BIN_LT_EQ: return "<=";
        case AST_BIN_GT_EQ: return ">=";
        case AST_BIN_LT:    return "<";
        case AST_BIN_GT:    return ">";
        default: return "";
    }
}

inline static char* sprint_annot(arena_t* a, ast_annot_t* v) {
    if( v->childcount > 0 ) {
        char* res = sprint_annot(a, v->children[0]);
        int count = v->childcount;
        for(int i = 1; i < count; i++) {
            res = asprintf(a, "%s, %s",
                res, sprint_annot(a, v->children[i]));
        }
        return asprintf(a, "%.*s<%s>",
            srcref_len(v->name),
            srcref_ptr(v->name),
            res);
    }
    return asprintf(a, "%.*s",
        srcref_len(v->name),
        srcref_ptr(v->name));
}

char* sprint_ast(arena_t* a, int ind, ast_node_t* n) {
    switch(n->type) {
        case AST_VAR_REF: {
            ast_varref_t v = n->u.n_varref;
            return asprintf(a, "%.*s",
                srcref_len(v.name),
                srcref_ptr(v.name));
        } break;
        case AST_ASSIGN: {
            ast_assign_t v = n->u.n_assign;
            return asprintf(a, "%s = %s",
                sprint_ast(a, ind, v.left_var),
                sprint_ast(a, ind, v.right_value));
        } break;
        case AST_VALUE: {
            ast_value_t v = n->u.n_value;
            switch(v.type) {
                case AST_VALUE_BOOL:    return asprintf(a, "%s", v.u._bool ? "true" : "false");
                case AST_VALUE_CHAR:    return asprintf(a, "%c", v.u._char);
                case AST_VALUE_FLOAT:   return asprintf(a, "%f", v.u._float);
                case AST_VALUE_INT:     return asprintf(a, "%d", v.u._int);
                case AST_VALUE_NONE:    return asprintf(a, "none");
                default:                return asprintf(a, "<unk>");
            }
        } break;
        case AST_FUN_CALL: {
            ast_funcall_t v = n->u.n_funcall;
            return asprintf(a, "%.*s(%s)",
                srcref_len(v.name), srcref_ptr(v.name),
                sprint_ast(a, ind, v.args));
        } break;
        case AST_ARGLIST: {
            ast_arglist_t v = n->u.n_args;
            int count = v.count;
            char* res = "";
            for(int i = 0; i < count; i++) {
                if( i > 0 ) {
                    res = asprintf(a,
                        "%s, %s", res,
                        sprint_ast(a, ind,
                            v.content[i]));
                } else {
                    res = sprint_ast(a, ind,
                            v.content[i]);
                }
            }
            return res;
        } break;
        case AST_FUN_DECL: {
            ast_fundecl_t v = n->u.n_fundecl;
            return asprintf(a, "%.*s(%s) %s",
                srcref_len(v.name), srcref_ptr(v.name),
                sprint_ast(a, ind, v.argspec),
                sprint_ast(a, ind+1, v.body));
        } break;
        case AST_FUN_EXDECL: {
            ast_funexdecl_t v = n->u.n_funexdecl;
            return asprintf(a, "%.*s(%s)",
                srcref_len(v.name), srcref_ptr(v.name),
                sprint_ast(a, ind, v.argspec));
        } break;
        case AST_BLOCK: {
            ast_block_t v = n->u.n_block;
            int count = v.count;
            if( count == 0 )
                return "{ }";
            char* res = asprintf(a, "{\n%*s",
                max(ind-1, 0), "");
            for(int i = 0; i < count; i++) {
                res = asprintf(a,
                    "%s%*s%s\n", res, ind + 1, "",
                    sprint_ast(a, ind,
                        v.content[i]));
            }
            res = asprintf(a, "%s%*s",
                res, ind, "");
            return asprintf(a, "%s}", res);
        } break;
        case AST_ARRAY: {
            ast_array_t v = n->u.n_array;
            int count = v.count;
            if( count == 0 )
                return "[]";
            char* res = sprint_ast(a, ind, v.content[0]);;
            for(int i = 1; i < count; i++) {
                res = asprintf(a,
                    "%s, %s", res,
                    sprint_ast(a, ind,
                        v.content[i]));
            }
            return asprintf(a, "[%s]", res);
        } break;
        case AST_RETURN: {
            ast_return_t v = n->u.n_return;
            return asprintf(a, "return %s",
                sprint_ast(a, ind, v.result));
        } break;
        case AST_BREAK: {
            return asprintf(a, "break");
        } break;
        case AST_UNOP: {
            ast_unop_t v = n->u.n_unop;
            return asprintf(a, "%s%s",
                unop_name(v.type),
                sprint_ast(a, ind, v.inner));
        } break;
        case AST_BINOP: {
            ast_binop_t v = n->u.n_binop;
            return asprintf(a, "(%s %s %s)",
                sprint_ast(a, ind, v.left),
                binop_name(v.type),
                sprint_ast(a, ind, v.right));
        } break;
        case AST_FOREACH: {
            ast_foreach_t v = n->u.n_foreach;
            return asprintf(a, "for(%s in %s) %s",
                sprint_ast(a, ind, v.vardecl),
                sprint_ast(a, ind, v.collection),
                sprint_ast(a, ind+1, v.during));
        } break;
        case AST_IF_CHAIN: {
            ast_node_t* current = n;
            char* res = "";
            int count = 0;
            while (current->type == AST_IF_CHAIN) {
                ast_if_t v = n->u.n_if;
                if( count == 0 ) {
                    res = asprintf(a, "if(%s) %s",
                        sprint_ast(a, ind, v.cond),
                        sprint_ast(a, ind+1, v.iftrue));
                } else {
                    res = asprintf(a, "%s else if(%s) %s",
                        res,
                        sprint_ast(a, ind, v.cond),
                        sprint_ast(a, ind+1, v.iftrue));
                }
                count ++;
                current = current->u.n_if.next;
            }
            if( ast_is_valid_else_block(current) ) {
                res = asprintf(a, "%s else %s",
                        res,
                        sprint_ast(a, ind+1, current));
            }
            return res;
        } break;
        case AST_TYANNOT: {
            ast_tyannot_t v = n->u.n_tyannot;
            return asprintf(a, "%s %s",
                sprint_annot(a, v.type),
                sprint_ast(a, ind, v.expr));
        } break;
        default: {
            return "<?>";
        }
    }
}


gvm_program_t gvm_program_compile_source(char* source, size_t source_len, char* filepath, bool debug_print) {

    parser_t parser = { 0 };
    trace_t trace = { 0 };

    if( trace_init(&trace, 16) == false ) {
        return (gvm_program_t) { 0 };
    }

    trace_set_current_source_path(&trace, filepath);

    arena_t* arena = arena_create(1024 * 500);
    pa_result_t result = pa_init(&parser, arena, &trace, source, source_len, filepath);
    if( par_is_error(result) ) {
        trace_fprint(stdout, &trace);
        trace_destroy(&trace);
        pa_destroy(&parser);
        return (gvm_program_t) { 0 };
    }
    result = pa_parse_program(&parser);

    if( par_is_error(result) ) {
        trace_fprint(stdout, &trace);
        trace_destroy(&trace);
        pa_destroy(&parser);
        return (gvm_program_t) { 0 };
    }

    if( par_is_nothing(result) ) {
        printf("error: empty program.\n");
        trace_fprint(stdout, &trace);
        pa_destroy(&parser);
        trace_destroy(&trace);
        return (gvm_program_t) { 0 };
    }

    ast_node_t* program_node = par_extract_node(result);
    
    if( debug_print ) {
        arena_t* arena = arena_create(512);
        printf("%s", sprint_ast(arena, 0, program_node));
        arena_destroy(arena);
    }

    gvm_program_t program = gvm_compile(arena, program_node, &trace);
    
    if( trace_get_error_count(&trace) > 0 ) {
        trace_fprint(stdout, &trace);
    }

    pa_destroy(&parser);
    arena_destroy(arena);
    return program;
}

gvm_program_t gvm_program_read_and_compile(char* path, bool debug_print) {

    FILE* f = fopen(path, "r");
    
    if( f == NULL ) {
        printf("error: %s not found.\n", path);
        return (gvm_program_t) { 0 };
    }

    char *source_text = malloc(1);
    int retry_counter = 100; 
    while( retry_counter > 0 ) {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        source_text = realloc(source_text, fsize + 1);
        if( fread(source_text, fsize, 1, f) > 0 ) {
            retry_counter = -10;
            source_text[fsize] = '\0';
        } else {
            usleep(100000);
            retry_counter --;
        }
    }

    fclose(f);

    if( retry_counter == 0 ) {
        printf("error: failed to read file: %s\n", path);
    }

    gvm_program_t program = gvm_program_compile_source(
        source_text,
        strlen(source_text),
        path,
        debug_print);

    free(source_text);

    return program;
}