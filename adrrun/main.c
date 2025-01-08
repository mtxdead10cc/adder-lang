#include <stdio.h>
#include <vm.h>
#include <sh_value.h>
#include <sh_ffi.h>
#include <vm_value_tools.h>
#include <sh_log.h>
#include <vm_heap.h>
#include <vm_env.h>
#include <co_program.h>
#include <co_ast.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <assert.h>
#include <sh_program.h>
#include "test/test_runner.h"
#include <sh_arena.h>
#include <sh_ift.h>
#include <xu_lib.h>


#define DEFAULT_PATH "resources/test.gvm"

val_t test(vm_t* vm, size_t argcount, val_t* args) {
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

void todo_list(void) {
    todo_item_t items[] = {        
        { false, "Foreach break." },
        { false, "FFI check separation\n"
                 "\t- the compiler should only set information about imports and exports to vm_program.\n"
                 "\t- additional check-pass is needed before executing VM to verify that the env supports the program.\n"}
    };
    size_t count = sizeof(items) / sizeof(items[0]);
    print_todo_list(items, count);
}

int main(int argv, char** argc) {

    char* path = NULL;
    bool disassemble = false;
    bool print_ast = false;
    bool print_help = false;
    bool keep_alive = false;
    bool run_tests = false;
    int path_arg = -1;

    todo_list();
    
    for(int i = 0; i < argv; i++) {
        disassemble |= strncmp(argc[i], "-d", 2) == 0;
        print_ast   |= strncmp(argc[i], "-a", 2) == 0;
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
        bool compile_ok = xu_quick_run(path, (xu_quickopts_t){
            disassemble, print_ast, keep_alive
        });
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
