#include "gvm.h"
#include "gvm_grid.h"
#include "gvm_parser.h"
#include "gvm_asm.h"
#include <stdio.h>

char* gvm_result_to_string(gvm_result_t res) {
    switch (res) {
        case RES_OK:            return "OK";
        case RES_OUT_OF_MEMORY: return "OUT OF MEMORY";
        case RES_ERROR:         return "PROCEDURE ERROR";
        default:                return "UNKNOWN";
    }
}

void gvm_print_if_error(gvm_result_t res, char* context) {
    if( res == RES_OK ) {
        return;
    }
    if( context == (char*) 0 ) {
        printf("error: %s\n", gvm_result_to_string(res));
    } else {
        printf("error: %s -- %s\n", context, gvm_result_to_string(res));
    }
}

bool pred(type_id_t init, type_id_t curr) {
    return init == curr;
}

void test() {
    grid_t grid;
    grid_init(&grid);
    grid_fill(&grid, 1);

    grid_print(&grid);
    grid_set_size(&grid, 5, 5);
    grid_print(&grid);

    grid_set_size(&grid, 5, 12);
    grid_print(&grid);

    int buf[ 5 * 12 ] = { 0 };
    int n = grid_select(&grid, 4, 4, pred, buf);
    printf("n selected: %i\n", n);
    for(int i = 0; i < n; i++) {
        grid.data[buf[i]] = 2;
    }
    grid_print(&grid);
    grid_destroy(&grid);

    char* str = "label:\n\tpush 5\n\tpush 0\n\tloop:"
        "\n\t\tdup 2\n\t\tis-less\n\t\tif-false exit-loop"
        "\n\t\tpush 1\n\t\tadd\n\t\tjump loop\n\texit-loop:\n\t\texit 0";
    printf("TEST \n%s\n", str);

    code_object_t obj = asm_assemble_code_object(str);
    asm_debug_disassemble_code_object(&obj);

    asm_destroy_code_object(&obj);
    
}