#include "co_program.h"
#include "co_parser.h"
#include "co_compiler.h"
#include "co_trace.h"
#include "sh_asminfo.h"
#include "sh_types.h"
#include "sh_value.h"
#include <unistd.h>
#include "sh_log.h"

#include <dlfcn.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

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
            res = asprint(a, "%s, %s",
                res, sprint_annot(a, v->children[i]));
        }
        return asprint(a, "%.*s<%s>",
            (int) srcref_len(v->name),
            srcref_ptr(v->name),
            res);
    }
    return asprint(a, "%.*s",
        (int) srcref_len(v->name),
        srcref_ptr(v->name));
}

char* sprint_ast(arena_t* a, int ind, ast_node_t* n) {
    switch(n->type) {
        case AST_VAR_REF: {
            ast_varref_t v = n->u.n_varref;
            return asprint(a, "%.*s",
                (int) srcref_len(v.name),
                srcref_ptr(v.name));
        } break;
        case AST_ASSIGN: {
            ast_assign_t v = n->u.n_assign;
            return asprint(a, "%s = %s",
                sprint_ast(a, ind, v.left_var),
                sprint_ast(a, ind, v.right_value));
        } break;
        case AST_VALUE: {
            ast_value_t v = n->u.n_value;
            switch(v.type) {
                case AST_VALUE_BOOL:    return asprint(a, "%s", v.u._bool ? "true" : "false");
                case AST_VALUE_CHAR:    return asprint(a, "%c", v.u._char);
                case AST_VALUE_FLOAT:   return asprint(a, "%f", v.u._float);
                case AST_VALUE_INT:     return asprint(a, "%d", v.u._int);
                case AST_VALUE_NONE:    return asprint(a, "none");
                default:                return asprint(a, "<unk>");
            }
        } break;
        case AST_FUN_CALL: {
            ast_funcall_t v = n->u.n_funcall;
            return asprint(a, "%.*s(%s)",
                (int) srcref_len(v.name), srcref_ptr(v.name),
                sprint_ast(a, ind, v.args));
        } break;
        case AST_ARGLIST: {
            ast_arglist_t v = n->u.n_args;
            int count = v.count;
            char* res = "";
            for(int i = 0; i < count; i++) {
                if( i > 0 ) {
                    res = asprint(a,
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
            return asprint(a, "%.*s(%s) %s",
                (int) srcref_len(v.name), srcref_ptr(v.name),
                sprint_ast(a, ind, v.argspec),
                sprint_ast(a, ind+1, v.body));
        } break;
        case AST_FUN_EXDECL: {
            ast_funexdecl_t v = n->u.n_funexdecl;
            return asprint(a, "%.*s(%s)",
                (int) srcref_len(v.name), srcref_ptr(v.name),
                sprint_ast(a, ind, v.argspec));
        } break;
        case AST_BLOCK: {
            ast_block_t v = n->u.n_block;
            int count = v.count;
            if( count == 0 )
                return "{ }";
            char* res = asprint(a, "{\n%*s",
                max(ind-1, 0), "");
            for(int i = 0; i < count; i++) {
                res = asprint(a,
                    "%s%*s%s\n", res, ind + 1, "",
                    sprint_ast(a, ind,
                        v.content[i]));
            }
            res = asprint(a, "%s%*s",
                res, ind, "");
            return asprint(a, "%s}", res);
        } break;
        case AST_ARRAY: {
            ast_array_t v = n->u.n_array;
            int count = v.count;
            if( count == 0 )
                return "[]";
            char* res = sprint_ast(a, ind, v.content[0]);;
            for(int i = 1; i < count; i++) {
                res = asprint(a,
                    "%s, %s", res,
                    sprint_ast(a, ind,
                        v.content[i]));
            }
            return asprint(a, "[%s]", res);
        } break;
        case AST_RETURN: {
            ast_return_t v = n->u.n_return;
            return asprint(a, "return %s",
                sprint_ast(a, ind, v.result));
        } break;
        case AST_BREAK: {
            return asprint(a, "break");
        } break;
        case AST_UNOP: {
            ast_unop_t v = n->u.n_unop;
            return asprint(a, "%s%s",
                unop_name(v.type),
                sprint_ast(a, ind, v.inner));
        } break;
        case AST_BINOP: {
            ast_binop_t v = n->u.n_binop;
            return asprint(a, "(%s %s %s)",
                sprint_ast(a, ind, v.left),
                binop_name(v.type),
                sprint_ast(a, ind, v.right));
        } break;
        case AST_FOREACH: {
            ast_foreach_t v = n->u.n_foreach;
            return asprint(a, "for(%s in %s) %s",
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
                    res = asprint(a, "if(%s) %s",
                        sprint_ast(a, ind, v.cond),
                        sprint_ast(a, ind+1, v.iftrue));
                } else {
                    res = asprint(a, "%s else if(%s) %s",
                        res,
                        sprint_ast(a, ind, v.cond),
                        sprint_ast(a, ind+1, v.iftrue));
                }
                count ++;
                current = current->u.n_if.next;
            }
            if( ast_is_valid_else_block(current) ) {
                res = asprint(a, "%s else %s",
                        res,
                        sprint_ast(a, ind+1, current));
            }
            return res;
        } break;
        case AST_TYANNOT: {
            ast_tyannot_t v = n->u.n_tyannot;
            return asprint(a, "%s %s",
                sprint_annot(a, v.type),
                sprint_ast(a, ind, v.expr));
        } break;
        default: {
            return "<?>";
        }
    }
}

bool program_file_exists(char *path) {
    if( path == NULL )
        return false;
    FILE *file = NULL;
    if ((file = fopen(path, "r"))) {
        fclose(file);
        return true;
    }
    return false;
}


time_t program_file_get_modtime(char *file_path) {
    struct stat attr;
    stat(file_path, &attr);
    return attr.st_mtim.tv_sec;
}

source_code_t program_source_from_memory(char* source_code, int length) {

    source_code_t src = (source_code_t) {
        .file_path = (char*) malloc( 16 * sizeof(char) ),
        .modtime = 0UL,
        .source_code = (char*) malloc( length * sizeof(char) ),
        .source_length = length   
    };

    // i'm fine with using goto for error handling in C
    if( src.file_path == NULL || src.source_code == NULL )
        goto alloc_failed;
    strncpy(src.file_path, "memory-buffer", 14);
    strncpy(src.source_code, source_code, length);
    return src;

alloc_failed:

    if( src.file_path != NULL )
        free(src.file_path);
    if( src.source_code != NULL )
        free(src.source_code);
    return (source_code_t) { 0 };
}

source_code_t program_source_read_from_file(char* file_path) {

    FILE* f = fopen(file_path, "r");
    
    if( f == NULL ) {
        sh_log_error("program_source_read_from_file: file not found\n\t%s", file_path);
        return (source_code_t) { 0 };
    }

    char *source_text = malloc(1);
    long source_length = 0L;
    int retry_counter = 100; 
    while( retry_counter > 0 ) {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        source_text = realloc(source_text, fsize + 1);
        if( fread(source_text, fsize, 1, f) > 0 ) {
            retry_counter = -10;
            source_text[fsize] = '\0';
            source_length = fsize;
        } else {
            usleep(100000);
            retry_counter --;
        }
    }

    fclose(f);

    if( retry_counter == 0 ) {
        sh_log_error("program_source_read_from_file: failed to read file\n\t%s", file_path);
        if( source_text != NULL )
            free(source_text);
        return (source_code_t) { 0 };
    }

    if( source_length <= 0 || source_length > INT32_MAX ) {
        sh_log_error("program_source_read_from_file: the file was too large "
            "(todo: fix source_code_t etc.)\n\t%s",
            file_path);
        if( source_text != NULL )
            free(source_text);
        return (source_code_t) { 0 };
    }

    size_t path_length = strnlen(file_path, 2048);
    char* path_clone = (char*) malloc((path_length + 1) * sizeof(char));

    if( path_length == 2048 || path_clone == NULL ) {
        sh_log_error("program_source_read_from_file: failed to create a copy of the file path\n");
        if( path_clone != NULL )
            free(path_clone);
        if( source_text != NULL )
            free(source_text);
        return (source_code_t) { 0 };
    }

    memset(path_clone, 0, (path_length + 1) * sizeof(char));
    strncpy(path_clone, file_path, path_length);

    return (source_code_t) {
        .file_path = path_clone,
        .modtime = program_file_get_modtime(file_path),
        .source_code = source_text,
        .source_length = (int) source_length
    };
}

bool program_source_is_valid(source_code_t* code) {
    if( code == NULL )
        return false;
    if( code->source_length <= 0 )
        return false;
    if( code->source_code == NULL )
        return false;
    if( code->file_path == NULL )
        return false;
    return true;
}

void program_source_free(source_code_t* code) {

    if( code == NULL )
        return;

    if( code->source_code != NULL )
        free(code->source_code);
    
    code->source_code = NULL;

    if( code->file_path != NULL )
        free(code->file_path);
    
    code->file_path = NULL;
}

program_t program_compile(source_code_t* code, bool print_ast) {

    parser_t parser = { 0 };
    trace_t trace = { 0 };

    if( program_source_is_valid(code) == false ) {
        sh_log_error("program_compile: received invalid source data");
        return (program_t) { 0 };
    }

    if( trace_init(&trace, 16) == false ) {
        sh_log_error("program_compile: failed to initialize trace");
        return (program_t) { 0 };
    }

    arena_t* arena = arena_create(1024 * 500);
    pa_result_t result = pa_init(&parser,
        arena,
        &trace,
        code->source_code,
        code->source_length,
        code->file_path);

    if( par_is_error(result) ) {
        define_cstr(str, 2048);
        trace_sprint(str, &trace);
        sh_log_error("PARSER\n%s", str);
        trace_destroy(&trace);
        pa_destroy(&parser);
        return (program_t) { 0 };
    }

    result = pa_parse_program(&parser);

    if( par_is_error(result) ) {
        define_cstr(str, 2048);
        trace_sprint(str, &trace);
        sh_log_error("PARSER\n%s", str);
        trace_destroy(&trace);
        pa_destroy(&parser);
        return (program_t) { 0 };
    }

    if( par_is_nothing(result) ) {
        define_cstr(str, 2048);
        trace_sprint(str, &trace);
        sh_log_error("PARSER\n%s", str);
        sh_log_error("the parser did not produce anything.");
        pa_destroy(&parser);
        trace_destroy(&trace);
        return (program_t) { 0 };
    }

    ast_node_t* program_node = par_extract_node(result);
    
    if( print_ast ) {
        arena_t* arena = arena_create(512);
        sh_log_info("DEBUG - AST\n%s\n",
            sprint_ast(arena, 0, program_node));
        arena_destroy(arena);
    }

    program_t program = gvm_compile(arena, program_node, &trace);
    
    if( trace_get_message_count(&trace) > 0 ) {
        define_cstr(str, 2048);
        trace_sprint(str, &trace);
        sh_log_error("COMPILER\n%s", str);
    }

    pa_destroy(&parser);
    arena_destroy(arena);
    trace_destroy(&trace);
    return program;
}