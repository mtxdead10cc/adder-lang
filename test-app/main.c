#include <stdio.h>
#include <gvm.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <gvm_value.h>
#include <gvm_env.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include "gvm_test.h"

time_t get_creation_time(char *path) {
    struct stat attr;
    stat(path, &attr);
    return attr.st_mtim.tv_sec;
}

#define DEFAULT_PATH "resources/test.gvm"

byte_code_block_t read_and_compile(char* path) {
    FILE* f = fopen(path, "r");
    if( f == NULL ) {
        printf("%s not found.\n", path);
        return (byte_code_block_t) { 0 };
    }

    char *asm_code = malloc(1);
    int retry_counter = 100; 
    while( retry_counter > 0 ) {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        asm_code = realloc(asm_code, fsize + 1);
        if( fread(asm_code, fsize, 1, f) > 0 ) {
            retry_counter = -10;
            asm_code[fsize] = '\0';
        } else {
            usleep(10000);
            retry_counter --;
        }
    }

    fclose(f);

    if( retry_counter == 0 ) {
        printf("failed to read file.\n");
    }

    byte_code_block_t obj = gvm_code_compile(asm_code);
    free(asm_code);

    return obj;
}

// OVERVIEW
//
// [Grid coordinates and grid vecs]
// 
// An integer vector 2 (x, y). Denoting a position within the grid.
// Is derived from grid element position.
// 
// [Grid element references]
// 
// Essentially an address pointing to a grid element.
// 
// [Grid element]
// 
// - current screen position
// - in play or not
// - accumulator (when merging with other elements)
// - element type
// 
// [Grid step]
// 
// 1. resolve grid integer coords
// 2. receive user input 
// 3. execute scripts -> generate grid operations & visual effects
// 4. apply grid operations & queue vfx
// 5. spawn new elements
// 6. animate movement
//
// 
// TODO
// [X] rename list -> array
// [X] add simple alloc and gc-dealloc for heap (array/list)
// [X] add support for VAL_IVEC2
// [ ] add support for function call frames
// [ ] update the grid code
// [ ] add support for grid operations
//     - ??? grid-ref <ivec> (push grid ref by lookup on ivec2)
//     - ??? grid-select <ivec> (push list of all flood fill refs with matching type)
//     - ...

bool run(char* path, bool verbose, bool keep_alive) {
    time_t last_creation_time = 0xFFFFFFFFFFFFFFFF;
    bool compile_ok = true;

    do {

        time_t creation_time = get_creation_time(path);

        if( creation_time <= last_creation_time ) {
            usleep(100);
            continue;
        }

        last_creation_time = creation_time;
        byte_code_block_t obj = read_and_compile(path);
        compile_ok = obj.size > 0;
        printf("%s [%s]\n\n", path, compile_ok ? "OK" : "FAILED");
        
        if( compile_ok ) {
            if( verbose ) {
                gvm_code_disassemble(&obj);
            }

            gvm_t vm = { 0 };
            gvm_create(&vm, 128, 128);

            val_t result = gvm_execute(&vm, &obj, 500);
            printf("\n> ");
            gvm_print_val(&vm, result);
            printf("\n");

            gvm_code_destroy(&obj);
            gvm_destroy(&vm);
        }

    } while ( keep_alive );

    return compile_ok;
}

int main(int argv, char** argc) {

    char* path = DEFAULT_PATH;
    bool verbose = false;
    bool print_help = false;
    bool keep_alive = false;
    bool run_tests = false;
    int path_arg = -1;
    

    for(int i = 0; i < argv; i++) {
        verbose     |= strncmp(argc[i], "-v", 2) == 0;
        print_help  |= strncmp(argc[i], "-h", 2) == 0;
        keep_alive  |= strncmp(argc[i], "-k", 2) == 0;
        run_tests   |= strncmp(argc[i], "-t", 2) == 0;
        int ext_pos      = strnlen(argc[i], 1024) - 4;
        if( path_arg >= 0 ) {
            continue;
        } else if( ext_pos <= 0 ) {
            continue;
        } else if( strncmp(((argc[i]) + ext_pos), ".gvm", 4) == 0 ) {
            path_arg = i;
        }
    }

    if( path_arg < 0 ) {
        path = DEFAULT_PATH;
    } else {
        path = argc[path_arg];
    }

    if( run_tests ) {
        printf("RUNNING TESTS\n");
        test_results_t result = run_testcases();
        int total = result.nfailed + result.npassed;
        printf("[%i / %i TESTS PASSED]\n", result.npassed, total);
    } else {
        bool compile_ok = run(path, verbose, keep_alive);
        print_help = print_help || (compile_ok == false && keep_alive == false);
    }

    if( print_help ) {
        printf( "usage: test-app <filename>"
        "\n\toptions:"
        "\n\t\t -v (verbose)"
        "\n\t\t -h (show this help message)"
        "\n\t\t -k (keep alive, reload and run on file update)"
        "\n" );
    }

    return 0;
}