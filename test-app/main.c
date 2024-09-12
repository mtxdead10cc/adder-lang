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
#include "board/board.h"
#include "board/termhax.h"

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
            usleep(100000);
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
// [ ] Implement board / grid

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
        byte_code_block_t obj = read_and_compile(path);
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

#define MIN(A,B) ((A) < (B) ? (A) : (B))
#define MAX(A,B) ((A) > (B) ? (A) : (B))

typedef struct tscr_t {
    char* title;
    thv2_t top_left;
    thv2_t size;
} tscr_t;

void tscr_draw_rect(thv2_t min, thv2_t max, int color) {
    int len = (max.x-min.x);
    char buf[len+2];
    for(int i = 0; i < len; i++) {
        buf[i] = ' ';
    }
    buf[len] = '\n';
    buf[len+1] = '\0';
    for(int y = min.y; y < max.y; y++) {
        termhax_set_pos((thv2_t){ min.x, y });
        termhax_print_color(buf, color);
    }
}

void tscr_draw_background(tscr_t* scr) {
    int title_w = strlen(scr->title);

    int top = scr->top_left.y;
    int left = scr->top_left.x;
    int bottom = scr->size.y + top;
    int right = scr->size.x + left;

    thv2_t center = thv2(
         left + ((right - left) / 2),
         top  + ((bottom - top) / 2));

    termhax_clear_screen();
    termhax_set_pos(thv2(MAX(center.x - (title_w / 2), 1), 1));
    printf("%s", scr->title);

    tscr_draw_rect(scr->top_left,
        thv2_add(scr->top_left, scr->size),
        COL_BG_YELLOW);
    
    termhax_set_pos(thv2(1, bottom));
    termhax_move_down(1);
    printf("min (%d, %d) max (%d,%d)\n", left, top, right, bottom);
}

void tscr_draw_cell(tscr_t* scr, int board_x, int board_y, int cell_width, char content, int color) {
    int inv_x = scr->top_left.x + (board_x * cell_width);
    int inv_y = scr->top_left.y + scr->size.y - board_y - 1;
    char buf[2] = { 0 };
    sprintf(buf, "%c", content);
    for(int i = 0; i < cell_width; i++) {
        termhax_set_pos(thv2(inv_x + i, inv_y));
        termhax_print_color(buf, color);
    }
}

void draw_board(board_t* board, int cursor_x, int cursor_y) {
    int cell_width = 2;
    tscr_t scr = {
        .size = thv2(board->dim[0] * cell_width, board->dim[1]),
        .title = "BOARD",
        .top_left = thv2(3, 3)
    };
    board_lookup_refresh(board);
    tscr_draw_background(&scr);
    for(int y = 0; y < board->dim[1]; y++) {
        for(int x = 0; x < board->dim[0]; x++) {
            piece_t* piece = board_lookup(board, x, y);
            if( piece == NULL ) {
                continue;
            }
            int color = COL_BG_MIN + (piece->type % COL_BG_COUNT);
            tscr_draw_cell(&scr, x, y, cell_width, ' ', color);
        }
    }
    tscr_draw_cell(&scr, cursor_x, cursor_y, cell_width, '#', COL_FG_MAGENTA);
    termhax_set_pos(thv2(1, board->dim[1] + 5));
    termhax_flush();
}

typedef enum keypress_t {
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_SELECT,
    KEY_QUIT,
    KEY_INCREASE_SIZE,
    KEY_DECREASE_SIZE,
    KEY_UNK
} keypress_t;


keypress_t read_key() {
    int inchar = termhax_getch();
    switch(termhax_to_upper(inchar)) {
        case 'A':    return KEY_LEFT;
        case 'W':    return KEY_UP;
        case 'S':    return KEY_DOWN;
        case 'D': return KEY_RIGHT;
        case 'Q': return KEY_QUIT;
        case '+': return KEY_INCREASE_SIZE;
        case '-': return KEY_DECREASE_SIZE;
        case ' ': return KEY_SELECT;
        default: return KEY_UNK;
    } 
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

    termhax_reserve_lines(50);

    board_t board = (board_t) { 0 };
    board_init(&board, 6, 8);

    bool quit = false;
    int cursor_x = board.dim[0] / 2;
    int cursor_y = board.dim[1] / 2;

    do {

        draw_board(&board, cursor_x, cursor_y);
        
        switch(read_key()) {
            case KEY_DOWN:
                cursor_y = MAX(0, cursor_y-1);
                break;
            case KEY_UP:
                cursor_y = MIN(board.dim[1], cursor_y+1);
                break;
            case KEY_LEFT:
                cursor_x = MAX(0, cursor_x-1);
                break;
            case KEY_RIGHT:
                cursor_x = MIN(board.dim[1], cursor_x+1);
                break;
            case KEY_SELECT:
                break;
            case KEY_INCREASE_SIZE:
                board_set_size(&board, board.dim[0]+1, board.dim[1]+1);
                break;
            case KEY_DECREASE_SIZE:
                board_set_size(&board, board.dim[0]-1, board.dim[1]-1);
                break;
            case KEY_UNK:
                printf("unknown key\n");
                break;
            case KEY_QUIT:
            default:
                quit = true;
                break;
        }
        
    } while (quit == false);
    
    return 0;
}