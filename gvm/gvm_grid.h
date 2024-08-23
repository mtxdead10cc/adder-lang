#ifndef GVM_GRID_H_
#define GVM_GRID_H_

#include "gvm_types.h"

typedef bool (grid_selcb_t)(type_id_t initial, type_id_t current);

#define GRID_CALC_INDEX(X, Y, W) (((Y) * (W)) + (X))
#define GRID_CALC_X(I, W) ((I) % (W))
#define GRID_CALC_Y(I, W) ((I) / (W))

// TODO: define all grid operations here
// NOTE: Handle VFX separately
// NOTE: the type_id_t stored in the grid are lookup indices
//       into the type definitions array

void grid_init(grid_t* grid);
void grid_set_size(grid_t* grid, int width, int height);
int  grid_select(grid_t* grid, int x, int y, grid_selcb_t* predicate, int* index_buffer);
void grid_fill(grid_t* grid, type_id_t val);
void grid_print(grid_t* grid);
void grid_destroy(grid_t* grid);

#endif