#include <stdio.h>
#include <gvm.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <gvm_value.h>
#include <gvm_env.h>

#define DEFAULT_PATH "resources/test.gvm"

int main(int argv, char** argc) {

    char* path = DEFAULT_PATH;
    bool verbose = false;
    bool print_help = argv < 2;
    int path_arg = -1;

    for(int i = 0; i < argv; i++) {
        verbose     |= strncmp(argc[i], "-v", 2) == 0;
        print_help  |= strncmp(argc[i], "-h", 2) == 0;
        int len      = strnlen(argc[i], 1024) - 4;
        if( path_arg >= 0 ) {
            continue;
        } else if( len <= 0 ) {
            continue;
        } else if( strncmp(argc[i], ".gvm", 4)) {
            path_arg = i;
        }
    }

    if( print_help ) {
        printf( "usage: test-app <filename>"
        "\n\toptions: -v (verbose)"
        "\n" );
    }

    if( path_arg < 0 ) {
        path = DEFAULT_PATH;
    }

    printf("continuing with %s\n", path);

    FILE* f = fopen(path, "r");
    if( f == NULL ) {
        printf("%s not found.\n", path);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *asm_code = malloc(fsize + 1);
    fread(asm_code, fsize, 1, f);
    fclose(f);

    asm_code[fsize] = 0;

    byte_code_block_t obj = gvm_compile(asm_code);

    free(asm_code);

    env_t env = { 0 };
    env_init(&env, &obj, 128);

    if( verbose ) {
        gvm_disassemble(&obj);
    }

    val_t result = gvm_execute(&obj, &env, 50);
    printf("> ");
    val_print_env(&env, &result);
    printf("\n");

    gvm_destroy(&obj);

    return 0;
}