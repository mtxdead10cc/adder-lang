#include <stdio.h>
#include <gvm.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <gvm_value.h>
#include <gvm_env.h>
#include <time.h>
#include <assert.h>
#include <gvm_heap.h>
#include <gvm_memory.h>
#include "board/board.h"
#include "utils/termhax.h"
#include "utils/keyboard.h"
#include "termgfx.h"

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
// TODO
// [ ] Implement board / grid

#define DEFAULT_PATH "scripts"

bool filter(piece_t* initial, piece_t* current) {
    if( initial->type == current->type ) {
        return true;
    }
    return false;
}

int main(int argv, char** argc) {

    char* path = DEFAULT_PATH;
    bool print_help = false;
    
    for(int i = 0; i < argv; i++) {
        print_help  |= strncmp(argc[i], "-h", 2) == 0;
    }

    if( print_help ) {
        printf( "usage: test-app"
        "\n\toptions:"
        "\n\t\t -h : show this help message"
        "\n" );
        return 0;
    }

    board_t board = (board_t) { 0 };
    query_result_t qres = (query_result_t) {0};

    board_init(&board, 6, 8);

    tscr_t screen = { 0 };

    tgfx_begin(&screen, "-- BOARD --");
    tgfx_set_size(&screen, board.dim[0], board.dim[1]);
    
    bool quit = false;
    int cursor_x = board.dim[0] / 2;
    int cursor_y = board.dim[1] / 2;

    do {

        int ncols = board.dim[0];
        int nrows = board.dim[1];

        board_lookup_refresh(&board);

        tgfx_begin_draw(&screen);

            for (int y = 0; y < nrows; y++) {
                for (int x = 0; x < ncols; x++) {
                    piece_t* piece = board_lookup(&board, x, y);
                    if( board_is_piece_in_play(piece) == false ) {
                        continue;
                    }
                    int color = COL_BG_MIN + (piece->type % COL_BG_COUNT);
                    tgfx_draw_cell(&screen, x, nrows-y-1, "  ", color);
                }
            }

            tgfx_draw_cell(&screen, cursor_x, nrows-cursor_y-1, "[]", COL_FG_MAGENTA);
            
        tgfx_end_draw(&screen);

        tgfx_clear_status_line(&screen, 0);
        printf("screen-size=(%d, %d) ", screen.size.x, screen.size.y);
        printf("board-pos=(%d, %d) ", cursor_x, nrows-cursor_y-1);
        
        switch(kb_read()) {
            case KEY_S:
            case KEY_ARROW_DOWN:
                cursor_y = MAX(0, cursor_y-1);
                break;
            case KEY_ARROW_UP:
            case KEY_W:
                cursor_y = MIN(board.dim[1]-1, cursor_y+1);
                break;
            case KEY_A:
            case KEY_ARROW_LEFT:
                cursor_x = MAX(0, cursor_x-1);
                break;
            case KEY_D:
            case KEY_ARROW_RIGHT:
                cursor_x = MIN(board.dim[0]-1, cursor_x+1);
                break;
            case KEY_SPACE: {
                board_query(&board, cursor_x, nrows-cursor_y-1, filter, &qres);
                tgfx_clear_status_line(&screen, 1);
                printf("\ncount: %i | ", qres.match_count);
                for (int i = 0; i < qres.match_count; i++) {
                    int index = qres.matches[i];
                    int x = index % board.dim[0];
                    int y = index / board.dim[0];
                    printf("(%d, %d) ", x, y);
                }
            } break;
            case KEY_PLUS:
                board_set_size(&board, board.dim[0]+1, board.dim[1]+1);
                tgfx_set_size(&screen, board.dim[0], board.dim[1]);
                break;
            case KEY_MINUS:
                board_set_size(&board, board.dim[0]-1, board.dim[1]-1);
                tgfx_set_size(&screen, board.dim[0], board.dim[1]);
                break;
            case KEY_UNK:
                printf("unknown key\n");
                break;
            case KEY_Q:
            default:
                quit = true;
                break;
        }
        
    } while (quit == false);

    tgfx_end(&screen, 2);
    
    return 0;
}