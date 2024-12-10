#include <stdio.h>
#include <vm.h>
#include <sh_value.h>
#include <sh_ffi.h>
#include <vm_value_tools.h>
#include <vm_heap.h>
#include <co_program.h>
#include <co_ast.h>
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

void adr_print(ffi_hndl_meta_t md, int argcount, val_t* args) {
    for(int i = 0; i < argcount; i++) {
        if( i > 0 )
            printf(" ");
        gvm_print_val(md.vm, args[i]);
    }
}

void setup_default_env(ffi_bundle_t* bundle) {
    bool res = ffi_bundle_init(bundle, 8);
    if( res == false ) {
        printf("error: failed to init FFI.\n");
        return;
    }
    res = ffi_bundle_add(bundle,
        sstr("print"), 
        (ffi_handle_t) {
            .local = 0,
            .tag = FFI_HNDL_ACTION,
            .u.action = adr_print,
        },
        ffi_vfunc(ffi_void(),
            ffi_list(ffi_char())));
    if( res == false ) {
        printf("error: failed to register FFI function: print\n");
    }
}

bool run(char* path, bool disassemble, bool show_ast, bool keep_alive) {
    time_t last_creation_time = 0xFFFFFFFFFFFFFFFF;
    bool compile_ok = true;
    ffi_bundle_t bundle = { 0 };
    setup_default_env(&bundle);

    do {

        time_t creation_time = get_creation_time(path);

        if( creation_time <= last_creation_time ) {
            usleep(100);
            continue;
        }

        last_creation_time = creation_time;
        gvm_program_t program = gvm_program_read_and_compile(path, show_ast, &bundle);
        compile_ok = program.inst.size > 0;
        printf("%s [%s]\n", path, compile_ok ? "OK" : "FAILED");
        
        if( compile_ok ) {

            if( disassemble ) {
                gvm_program_disassemble(stdout, &program);
            }

            gvm_t vm = { 0 };
            gvm_create(&vm, 128, 128);

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

    ffi_bundle_destroy(&bundle);

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
        { false, "Foreach break." }
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
        bool compile_ok = run(path, disassemble, print_ast, keep_alive);
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