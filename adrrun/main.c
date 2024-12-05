#include <stdio.h>
#include <vm.h>
#include <sh_value.h>
#include <vm_env.h>
#include <vm_value_tools.h>
#include <vm_heap.h>
#include <co_program.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>
#include <sh_program.h>
#include "test/test_runner.h"
#include <sh_arena.h>

time_t get_creation_time(char *path) {
    struct stat attr;
    stat(path, &attr);
    return attr.st_mtim.tv_sec;
}

#define DEFAULT_PATH "resources/test.gvm"

val_t test(gvm_t* vm, size_t argcount, val_t* args) {
    (void)(argcount); // hide unused warning
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
        gvm_program_t program = gvm_program_read_and_compile(path);
        compile_ok = program.inst.size > 0;
        printf("%s [%s]\n", path, compile_ok ? "OK" : "FAILED");
        
        if( compile_ok ) {

            if( verbose ) {
                gvm_program_disassemble(stdout, &program);
            }

            gvm_t vm = { 0 };
            gvm_create(&vm, 128, 128);

            // register native functions
            gvm_native_func(&vm, "test", "arr", 2, &test);

            // execute script
            gvm_exec_args_t args = { 0 };
            args.cycle_limit = 500;
            val_t result = gvm_execute(&vm, &program, &args);

            printf("> ");
            gvm_print_val(&vm, result);
            printf("\n");

            gvm_program_destroy(&program);
            gvm_destroy(&vm);
        }

    } while ( keep_alive );

    return compile_ok;
}

typedef struct todo_item_t {
    bool is_done;
    char* descr;
} todo_item_t;

void print_todo_list(todo_item_t* items, size_t count) {
    printf("TODO LIST\n");
    printf("--------------------------------------\n");
    for(size_t i = 0; i < count; i++) {
        printf(" [%s] %s\n",
            items[i].is_done ? "X" : " ",
            items[i].descr);
    }
    printf("--------------------------------------\n");
}

void todo_list() {
    todo_item_t items[] = {        
        { false, "VM execute must verify that all required " 
                 "native_funsign_t requirenments are fulfilled." },
        { true, "Compiler checks return type on funcall." },
        { false, "Foreach break." },
        { true, "Typechecker: figure out how to have var declarations without assignments."},
        { true,  "Improve the typechecking (maybe use ast_annot_t instead of strings)." }
    };
    size_t count = sizeof(items) / sizeof(items[0]);
    print_todo_list(items, count);
}

int main(int argv, char** argc) {

    char* path = NULL;
    bool verbose = false;
    bool print_help = false;
    bool keep_alive = false;
    bool run_tests = false;
    int path_arg = -1;

    todo_list();
    
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
        } else if( strncmp(((argc[i]) + ext_pos), ".adr", 4) == 0 ) {
            path_arg = i;
        }
    }

    if( path_arg >= 0 ) {
        path = argc[path_arg];
    }

    if( path != NULL ) {
        bool compile_ok = run(path, verbose, keep_alive);
        print_help = print_help || (compile_ok == false && keep_alive == false);
    }

    if( run_tests || path == NULL ) {
        printf("RUNNING TESTS\n");
        test_results_t result = run_testcases();
        int total = result.nfailed + result.npassed;
        printf("[%i / %i TESTS PASSED]\n", result.npassed, total);
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