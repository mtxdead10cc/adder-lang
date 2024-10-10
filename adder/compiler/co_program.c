#include "co_program.h"
#include "co_parser.h"
#include "co_compiler.h"
#include "sh_asminfo.h"
#include "sh_types.h"
#include "sh_value.h"
#include <unistd.h>

gvm_program_t gvm_program_compile_source(char* source, size_t source_len, char* filepath) {

    parser_t parser = { 0 };
    pa_result_t result = pa_init(&parser, source, source_len, filepath);
    if( par_is_error(result) ) {
        cres_fprint(stdout, (cres_t*) par_extract_error(result), filepath);
        pa_destroy(&parser);
        return (gvm_program_t) { 0 };
    }
    result = pa_parse_program(&parser);
    if( par_is_error(result) ) {
        cres_fprint(stdout, (cres_t*) par_extract_error(result), filepath);
        pa_destroy(&parser);
        return (gvm_program_t) { 0 };
    }
    if( par_is_nothing(result) ) {
        printf("error: empty program.\n");
        pa_destroy(&parser);
        return (gvm_program_t) { 0 };
    }

    ast_node_t* program_node = par_extract_node(result);
    cres_t status = {0};
    gvm_program_t program = gvm_compile(program_node, &status);
    if( cres_has_error(&status) ) {
        cres_fprint(stdout, &status, filepath);
    }
    ast_free(program_node);
    pa_destroy(&parser);
    return program;
}

gvm_program_t gvm_program_read_and_compile(char* path) {

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
        path);

    free(source_text);

    return program;
}