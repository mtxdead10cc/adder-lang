#include "board.h"
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

void board_set_dirty(board_t* board) {
    board->mod_cntr ++;
}

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
            .type = (i / BOARD_MAX_ROWS) + 1
        };
    }
    board_set_dirty(board);
    board_lookup_refresh(board);
}

bool board_is_piece_in_play(piece_t* piece) {
    if( piece == NULL ) {
        return false;
    }
    return piece->layer == 0
        && piece->pos[0] >= 0.0f
        && piece->pos[0] <= 1.0f
        && piece->pos[1] >= 0.0f
        && piece->pos[1] <= 1.0f;
}

piece_t* board_lookup(board_t* board, int x, int y) {
    if( x < 0 || y < 0 ) {
        return NULL;
    }
    if( x >= board->dim[0] || y >= board->dim[1] ) {
        return NULL;
    }
    int lut_index = (y * board->dim[0]) + x;
    int piece_index = board->lookup.index[lut_index];
    if( piece_index < 0 ) {
        return NULL;
    }
    return &board->pieces[piece_index];
}

void board_lookup_refresh(board_t* board) {
    if( board->lookup.mod_cntr == board->mod_cntr ) {
        return;
    }
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
    board->lookup.mod_cntr = board->mod_cntr;
}

#define _MIN(A,B) ((A) < (B) ? (A) : (B))
#define _MAX(A,B) ((A) > (B) ? (A) : (B))
#define _CLAMP(V,MIN,MAX) _MAX(_MIN((V), (MAX)), (MIN))

void board_set_size(board_t* board, int ncols, int nrows) {
    ncols = _CLAMP(ncols, 1, BOARD_MAX_COLUMNS);
    nrows = _CLAMP(nrows, 1, BOARD_MAX_ROWS);
    float x_p = (float) board->dim[0] / (float) ncols;
    float y_p = (float) board->dim[1] / (float) nrows;
    for(int i = 0; i < BOARD_MAX_PIECES; i++) {
        board->pieces[i].pos[0] *= x_p;
        board->pieces[i].pos[1] *= y_p;
    }
    board->dim[0] = ncols;
    board->dim[1] = nrows;
    board_set_dirty(board);
    board_lookup_refresh(board);
}

bool query_is_visited(query_result_t* result, int index) {
    for(int i = 0; i < result->visited_count; i++) {
        if( result->visited[i] == index ) {
            return true;
        }
    }
    return false;
}

void query_add_visited(query_result_t* result, int index) {
    result->visited[result->visited_count++] = index;
    assert(result->visited_count < BOARD_MAX_PIECES);
}

void query_add_match(query_result_t* result, int index) {
    result->matches[result->match_count++] = index;
    assert(result->match_count < BOARD_MAX_PIECES);
}

void query_check(board_t* board, piece_t* initial, int x, int y, query_filter_t filter, query_result_t* result) {
    if( query_is_visited(result, (y * board->dim[0]) + x) ) {
        return;
    }
    piece_t* other = board_lookup(board, x, y);
    if( board_is_piece_in_play(other) == false ) {
        return;
    }
    query_add_visited(result, (y * board->dim[0]) + x);
    if( filter(initial, other) == false ) {
        return;
    }
    query_add_match(result, (y * board->dim[0]) + x);
    query_check(board, initial, x+1, y, filter, result);
    query_check(board, initial, x-1, y, filter, result);
    query_check(board, initial, x, y+1, filter, result);
    query_check(board, initial, x, y-1, filter, result);
}

int board_query(board_t* board, int x, int y, query_filter_t filter, query_result_t* result) {
    board_lookup_refresh(board);
    piece_t* initial = board_lookup(board, x, y);
    if( initial == NULL ) {
        return 0;
    }
    result->match_count = 0;
    result->visited_count = 0;
    query_check(board, initial, x, y, filter, result);
    return result->match_count;
}