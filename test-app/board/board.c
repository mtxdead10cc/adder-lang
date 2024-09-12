#include "board.h"
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

void board_init(board_t* board, int ncols, int nrows) {
    board->dim[0] = ncols;
    board->dim[1] = nrows;
    board->draw_size[0] = 1.0f;
    board->draw_size[1] = 1.0f;
    for(int i = 0; i < BOARD_MAX_PIECES; i++) {
        float percent_x = ((float) (i % BOARD_MAX_COLUMNS) + 0.5f) / (float) board->dim[0];
        float percent_y = ((float) (i / BOARD_MAX_COLUMNS) + 0.5f) / (float) board->dim[1];
        board->pieces[i] = (piece_t) {
            .layer = (percent_x <= 1.0f && percent_y <= 1.0f) ? 0 : 1,
            .pos = {
                percent_x,
                percent_y
            },
            .type = i % 5
        };
    }
}

bool board_is_piece_in_play(piece_t* piece) {
    return piece->layer == 0
        && piece->pos[0] >= 0.0f
        && piece->pos[0] <= 1.0f
        && piece->pos[1] >= 0.0f
        && piece->pos[1] <= 1.0f;
}

piece_t* board_lookup(board_t* board, int x, int y) {
    int lut_index = (y * board->dim[0]) + x;
    int count = board->dim[0] * board->dim[1];
    if( lut_index >= count ) {
        return NULL;
    }
    int piece_index = board->lookup.index[lut_index];
    if( piece_index < 0 ) {
        return NULL;
    }
    return &board->pieces[piece_index];
}

void board_lookup_refresh(board_t* board) {
    // clear LUT
    for(int i = 0; i < BOARD_MAX_PIECES; i++) {
        board->lookup.index[i] = -1;
    }
    // set indices
    for(int i = 0; i < BOARD_MAX_PIECES; i++) {
        if( board_is_piece_in_play(&board->pieces[i]) == false ) {
            continue;
        }
        float* pos = board->pieces[i].pos;
        int x = pos[0] * board->dim[0];
        int y = pos[1] * board->dim[1];
        assert(board->lookup.index[(y*board->dim[0]) + x] == -1);
        board->lookup.index[(y*board->dim[0]) + x] = i;
    }
}

void board_set_size(board_t* board, int ncols, int nrows) {
    float x_p = (float) board->dim[0] / (float) ncols;
    float y_p = (float) board->dim[1] / (float) nrows;
    for(int i = 0; i < BOARD_MAX_PIECES; i++) {
        board->pieces[i].pos[0] *= x_p;
        board->pieces[i].pos[1] *= y_p;
    }
    board->dim[0] = ncols;
    board->dim[1] = nrows;
    board_lookup_refresh(board);
}