#ifndef TERMGFX_H_
#define TERMGFX_H_

#include "utils/termhax.h"

typedef struct tscr_t {
    char* title;
    int title_width;
    thv2_t top_left;
    thv2_t size;
} tscr_t;

#define TGFX_CELL_WIDTH 2

void tgfx_begin(tscr_t* screen, char* title);
void tgfx_set_size(tscr_t* screen, int height, int width);
void tgfx_begin_draw(tscr_t* screen);
void tgfx_draw_cell(tscr_t* screen, int x, int y, char* content, int color);
void tgfx_end_draw(tscr_t* screen);
void tgfx_end(tscr_t* screen, int status_lines);
void tgfx_clear_status_line(tscr_t* screen, int line);

#endif // TERMGFX_H_