#include "gvm_grid.h"
#include "gvm_config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(val, min, max) (MAX(MIN((val), (max)), (min)))

void grid_init(grid_t* grid) {
    grid->data = (type_id_t*) malloc(GRID_INITIAL_HEIGHT * GRID_INITIAL_WIDTH);
    if( grid->data == NULL ) {
        printf("error: failed to allocate grid of size (%i, %i).\n",
            GRID_INITIAL_WIDTH, GRID_INITIAL_HEIGHT);
        return;
    }
    grid->height = GRID_INITIAL_HEIGHT;
    grid->width = GRID_INITIAL_WIDTH;
    memset(grid->data, 0, grid->width * grid->height);
}

void grid_set_size(grid_t* grid, int width, int height) {

    type_id_t* new_grid = (type_id_t*) malloc(width * height);
    if( new_grid == NULL ) {
        printf("error: failed to allocate grid of size (%i, %i).\n",
            width, height);
        return;
    }

    memset(new_grid, 0, width * height);

    int cw = MIN(width, grid->width);
    int ch = MIN(height, grid->height);
    
    for(int x = 0; x < cw; x++) {
        for(int y = 0; y < ch; y++) {
            int i_dest = GRID_CALC_INDEX(x, y, width);
            int i_src = GRID_CALC_INDEX(x, y, grid->width);
            new_grid[i_dest] = grid->data[i_src];
        }
    }

    grid->height = height;
    grid->width = width;
    free(grid->data);
    grid->data = new_grid;
}

void grid_print(grid_t* grid) {
    for (int x = 0; x < grid->width; x++) {
        printf("-----");
    }
    printf("\n");
    for (int y = grid->height - 1; y >= 0; y--) {
        for (int x = 0; x < grid->width; x++) {
            printf("0x%02X ", (int) grid->data[GRID_CALC_INDEX(x, y, grid->width)]);
        }
        printf("\n");
    }
    for (int x = 0; x < grid->width; x++) {
        printf("-----");
    }
    printf("\n");
}

void grid_fill(grid_t* grid, type_id_t val) {
    for(int i = 0; i < (grid->height * grid->width); i++) {
        grid->data[i] = val;
    }
}

typedef struct select_data_t {
    grid_t* grid;
    type_id_t initial;
    int* visited;
    int visited_count;
    grid_selcb_t* predicate;
    int* selection;
    int selected_count;
} select_data_t;

bool _is_visited(select_data_t* select, int value) {
    int count = select->visited_count;
    int* array = select->visited;
    for(int i = count - 1; i >= 0; i--) {
        if( array[i] == value ) {
            return true;
        }
    }
    return false;
}

int _try_select(select_data_t* select, int x, int y) {
    int w = select->grid->width;
    int h = select->grid->height;
    x = CLAMP(x, 0, w-1);
    y = CLAMP(y, 0, h-1);
    int current_index = GRID_CALC_INDEX(x, y, w);
    if( _is_visited(select, current_index) ) {
        return 0;
    }
    select->visited[select->visited_count++] = current_index;
    type_id_t initial = select->initial;
    type_id_t current = select->grid->data[current_index];
    if( select->predicate(initial, current) == false ) {
        return 0;
    }
    select->selection[select->selected_count++] = current_index;
    int added = 1;
    added += _try_select(select, x-1, y);
    added += _try_select(select, x+1, y);
    added += _try_select(select, x, y-1);
    added += _try_select(select, x, y+1);
    return added;
}

int grid_select(grid_t* grid, int x, int y, grid_selcb_t* predicate, int* index_buffer) {
    int w = grid->width;
    int h = grid->height;
    x = CLAMP(x, 0, w-1);
    y = CLAMP(y, 0, h-1);
    select_data_t select = {
        .grid = grid,
        .initial = grid->data[GRID_CALC_INDEX(x, y, w)],
        .predicate = predicate,
        .selected_count = 0,
        .selection = index_buffer,
        .visited = (int*) malloc(h * w * sizeof(int)),
        .visited_count = 0
    };
    _try_select(&select, x, y);
    free(select.visited);
    return select.selected_count;
}

void grid_destroy(grid_t* grid) {
    free(grid->data);
    grid->height = 0;
    grid->width = 0;
    grid->data = NULL;
}