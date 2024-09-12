#ifndef BOARD_H_
#define BOARD_H_

#include <stdint.h>
#include <stdbool.h>

#define BOARD_MAX_COLUMNS 16
#define BOARD_MAX_ROWS 24
#define BOARD_MAX_PIECES (BOARD_MAX_COLUMNS * BOARD_MAX_ROWS)

typedef struct piece_t {
    float pos[2]; // normalized screen position
    int layer;    // layer (0 to N) is the piece in play etc.
    int type;     // the type of the piece
} piece_t;

typedef struct lut_t {
    uint32_t turn;  // the turn this lut was generated for
    int index[BOARD_MAX_PIECES]; // indices for lookup by position
} lut_t;

typedef struct board_t {
    float draw_size[2]; // draw_size[0] = <total width>, draw_size[1] = <total height>
    int dim[2];         // dim[0] = <num cols>, dim[1] = <num rows>
    uint32_t turn;      // the current turn (0 to N) 
    piece_t pieces[BOARD_MAX_PIECES]; // the pieces of the board
    lut_t lookup;
} board_t;

void  board_init(board_t* board, int ncols, int nrows);
bool  board_is_piece_in_play(board_t* board, piece_t* piece);
void  board_lookup_refresh(board_t* board);
piece_t* board_lookup(board_t* board, int x, int y);
bool  board_set_size(board_t* board, int ncols, int nrows);

#endif // BOARD_H_