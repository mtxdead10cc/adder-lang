#include "adrcom/compiler/co_program.h"
#include "adrcom/compiler/co_compiler.h"

#include <adrcom/shared/co_trace.h>

#include <adrcom/ast/co_ast.h>

#include <adrcom/parser/co_parser.h>

#include <shared/sh_asminfo.h>
#include <shared/sh_types.h>
#include <shared/sh_value.h>
#include <shared/sh_log.h>
#include <shared/sh_arena.h>
#include <shared/sh_json.h>

#include <unistd.h>

#include <dlfcn.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

char* unop_name(ast_t* n) {
    // TODO:
    // these should match the source code symbols
    // in the language itself
    switch(n->tag) {
        case AST_UNA_NEG: return "-";
        case AST_UNA_NOT: return "not";
        default: return "";
    }
}

char* binop_name(ast_t* n) {
    // TODO:
    // these should match the source code symbols
    // in the language itself
    switch(n->tag) {
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

inline static char* sprint_type_descriptor(arena_t* a, ast_t* v) {
    srcref_t name = ast_try_get_name(v);
    if( v->size > 0 ) {
        char* res = sprint_type_descriptor(a, v->as.items[0]);
        int count = v->size;
        for(int i = 1; i < count; i++) {
            res = asprint(a, "%s, %s",
                res, sprint_type_descriptor(a, v->as.items[i]));
        }
        return asprint(a, "%.*s<%s>",
            (int) srcref_len(name),
            srcref_ptr(name),
            res);
    }
    return asprint(a, "%.*s",
        (int) srcref_len(name),
        srcref_ptr(name));
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
        .modtime = 0UL,
        .source = src_create("memory-buffer", source_code, length)
    };

    if( src.source == NULL )
        return (source_code_t) {0};

    return src;
}

source_code_t program_source_read_from_file(char* file_path) {

    src_t* source = src_load(file_path);

    if(source == NULL) {
        sh_log_error("program_source_read_from_file: failed to load source");
        return (source_code_t) {0};
    }

    return (source_code_t) {
        .modtime = program_file_get_modtime(file_path),
        .source = source
    };
}

bool program_source_is_valid(source_code_t* code) {
    if( code == NULL )
        return false;
    if( code->source == NULL )
        return false;
    return true;
}

void program_source_free(source_code_t* code) {
    if( code == NULL )
        return;
    src_destroy(code->source);
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
        code->source);

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

    ast_t* program_node = par_extract_node(result);
    
    if( print_ast ) {
        json_value_t* value = ast_to_json(arena, program_node);
        char* str = json_dumps(value, 2);
        sh_log_info("DEBUG - AST\n%s\n", str);
        free(str);
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