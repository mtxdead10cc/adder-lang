#include "board/board.h"
#include "utils/termhax.h"
#include "utils/keyboard.h"
#include "termgfx.h"

void tscr_draw_rect(thv2_t min, thv2_t max, int color) {
    int len = (max.x-min.x) * TGFX_CELL_WIDTH;
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

void tscr_draw_background(tscr_t* screen) {

    int top = screen->top_left.y;
    int bottom = screen->size.y + top;

    termhax_clear_screen_above_cursor(bottom + 1);
    
    int w = (screen->size.x * TGFX_CELL_WIDTH) / 2;
    w += screen->top_left.x;
    w -= screen->title_width / 2;
    termhax_set_pos(thv2(w, 1));
    printf("%s", screen->title);

    tscr_draw_rect(screen->top_left,
        thv2_add(screen->top_left, screen->size),
        COL_BG_YELLOW);
    
    termhax_set_pos(thv2(0, bottom));
    termhax_move_down(1);
}

int tgfx_bottom_line(tscr_t* screen) {
    return screen->top_left.y + screen->size.y;
}

void tgfx_begin(tscr_t* screen, char* title) {
    screen->size = thv2(3, 3);
    screen->title = title;
    screen->title_width = strlen(screen->title);
    screen->top_left = thv2(3, 3);
    termhax_reserve_lines(tgfx_bottom_line(screen)+5);
    termhax_clear_screen();
}

void tgfx_begin_draw(tscr_t* screen) {
    tscr_draw_background(screen);
}

void tgfx_draw_cell(tscr_t* screen, int x, int y, char* content, int color) {
    char buf[TGFX_CELL_WIDTH+2] = { 0 };
    thv2_t pos = screen->top_left;
    pos = thv2_add(pos, thv2(x * TGFX_CELL_WIDTH, y));
    snprintf(buf, TGFX_CELL_WIDTH+1, "%s", content);
    termhax_set_pos(pos);
    termhax_print_color(buf, color);
}

void tgfx_end_draw(tscr_t* screen) {
    termhax_set_pos(thv2(0, tgfx_bottom_line(screen) + 1));
    termhax_flush();
}

void tgfx_end(tscr_t* screen, int status_lines) {
    termhax_set_pos(thv2(0, tgfx_bottom_line(screen) + 1 + status_lines));
    termhax_flush();
}

void tgfx_clear_status_line(tscr_t* screen, int line) {
    termhax_clear_line(tgfx_bottom_line(screen) + 1 + line);
}

void tgfx_set_size(tscr_t* screen, int width, int height) {
    termhax_clear_screen();
    screen->size = thv2(width, height);
}
