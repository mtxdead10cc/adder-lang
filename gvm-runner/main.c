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
#include <string.h>
#include <assert.h>
#include <gvm_heap.h>
#include <gvm_memory.h>
#include "test/test_runner.h"

time_t get_creation_time(char *path) {
    struct stat attr;
    stat(path, &attr);
    return attr.st_mtim.tv_sec;
}

#define DEFAULT_PATH "resources/test.gvm"

val_t test(gvm_t* vm, val_t* args) {
    int a = val_into_number(args[0]);
    int b = val_into_number(args[1]);
    int len = b - a + 1;
    len = (len >= 0) ? len : 0;
    array_t array = heap_array_alloc(vm, len);
    val_t* ptr = array_get_ptr(vm, array, 0);
    for(int i = 0; i < len; i++) {
        ptr[i] = val_number(a + i);
    }
    return val_array(array);
}

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
        gvm_byte_code_t obj = gvm_read_and_compile(path);
        compile_ok = obj.size > 0;
        printf("%s [%s]\n\n", path, compile_ok ? "OK" : "FAILED");
        
        if( compile_ok ) {

            if( verbose ) {
                gvm_code_disassemble(&obj);
            }

            gvm_t vm = { 0 };
            gvm_create(&vm, 128, 128);

            // register native functions
            gvm_native_func(&vm, "test", 2, &test);

            // execute script
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
        "\n\t\t -v : verbose output"
        "\n\t\t -h : show this help message"
        "\n\t\t -k : keep alive, reload and run on file update"
        "\n\t\t -t : run test cases"
        "\n" );
    }

    return 0;
}