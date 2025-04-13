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

bool is_adr_path(char* str) {
    int len = strnlen(str, 1024);
    len = str_rstrip_whitespace(str, len) - 4;
    if( len <= 0 )
        return false;
    return strncmp((str + len), ".adr", 4) == 0;
}

bool is_adr_call(char* str) {
    int len = strnlen(str, 1024);
    len = str_rstrip_whitespace(str, len) - 1;
    if( len <= 0 )
        return false;
    return strncmp((str + len), ")", 1) == 0;
}


int main(int argv, char** argc) {

    char* path = NULL;
    char* callstr = NULL;
    int memory = 1024;
    bool disassemble = false;
    bool print_ast = false;
    bool print_help = false;
    bool keep_alive = false;
    bool run_tests = false;
    int path_arg = -1;
    int ep_arg = -1;
    int mem_arg = -1;
    
    for(int i = 0; i < argv; i++) {

        disassemble |= strncmp(argc[i], "-d", 2) == 0;
        print_ast   |= strncmp(argc[i], "-a", 2) == 0;
        print_help  |= strncmp(argc[i], "-h", 2) == 0;
        keep_alive  |= strncmp(argc[i], "-k", 2) == 0;
        run_tests   |= strncmp(argc[i], "-t", 2) == 0;

        if( is_adr_path(argc[i]) )
            path_arg = i;

        if( is_adr_call(argc[i]) )
            ep_arg = i;

        if( strncmp(argc[i], "-m=", 3) == 0 )
            mem_arg = i;
    }

    if( path_arg >= 0 ) {
        path = argc[path_arg];
    }

    if( ep_arg >= 0 ) {
        callstr = argc[ep_arg];
    }

    if( mem_arg >= 0 ) {
        int len = strnlen(argc[mem_arg], (3+11));
        if( len > 0 && len <= (3+10) ) {
            sscanf(argc[mem_arg]+3, "%d", &memory);
        }
    }

    if( path != NULL ) {
        xu_quick_run(path, (xu_quickopts_t) {
            disassemble, print_ast, 
            keep_alive, memory, callstr 
        });
    } else {
        print_help = true;
    }

    if( run_tests || path == NULL ) {
        sh_log_info("RUNNING TESTS\n");
        test_results_t result = run_testcases();
        int total = result.nfailed + result.npassed;
        sh_log_info("[%i / %i TESTS PASSED]\n", result.npassed, total);
    }

    if( print_help ) {
        sh_log_info(
        "\n\tusage: adrrun <filename>"
        "\n\toptions:"
        "\n\t\t -v     : verbose output"
        "\n\t\t -h     : show this help message"
        "\n\t\t -k     : keep alive, reload and run on file update"
        "\n\t\t -t     : run test cases"
        "\n\t\t -a     : show ast"
        "\n\t\t -d     : show disassembly"
        "\n\t\t -m=<n> : specify VM total memory (value count)"
        "\n" );
    }

    return 0;
}
