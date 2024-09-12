#ifndef TERMHAX_H_
#define TERMHAX_H_

#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <termios.h>
#include <unistd.h>

#define MOVE_UP "\033[A"
#define MOVE_DOWN "\033[X"
#define ERASE_LINE "\33[2K"
#define LINE_RETURN "\r"

typedef struct thv2_t {
    int x;
    int y;
} thv2_t;



int termhax_getch() { 
    int ch;
    struct termios oldattr, newattr;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~ICANON;
    newattr.c_lflag &= ~ECHO;
    newattr.c_cc[VMIN] = 1;
    newattr.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch; 
}

int termhax_to_upper(int c) {
    if( c >= 'a' && c <= 'z' ) {
        int offs = c - 'a';
        return 'A' + offs;
    } else {
        return c;
    }
}

inline static thv2_t thv2(int x, int y) {
    return (thv2_t) {
        .x = x,
        .y = y
    };
}

inline static thv2_t thv2_add(thv2_t a, thv2_t b) {
    return (thv2_t) {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
}

inline static void termhax_reserve_lines(int lines) {
    while( lines > 0 ) {
        printf("\n");
        lines --;
    }
}

inline static void termhax_flush() {
    fflush(stdout);
}

inline static void termhax_move_up(int lines) {
    printf("\033[%dA", lines);
}

inline static void termhax_move_down(int lines) {
    printf("\033[%dB", lines);
}

inline static void termhax_move_left(int cols) {
    printf("\033[%dD", cols); // Move left X column;
}

inline static void termhax_move_right(int cols) {
    printf("\033[%dC", cols); // Move right X column;
}

inline static void termhax_clear_screen() {
    printf("\033[2J");
}

inline static void termhax_set_pos(thv2_t v) {
    assert(v.y > 0 && v.x > 0); // minimum value is 1
    printf("\033[%d;%dH", v.y, v.x);
}

#define COL_FG_BLACK     30
#define COL_FG_RED       31
#define COL_FG_GREEN     32
#define COL_FG_YELLOW    33
#define COL_FG_BLUE      34
#define COL_FG_MAGENTA   35
#define COL_FG_CYAN      36
#define COL_FG_WHITE     37

#define COL_BG_MIN       40
#define COL_BG_MAX       47
#define COL_BG_COUNT (COL_BG_MAX - COL_BG_MIN)

#define COL_BG_BLACK     40
#define COL_BG_RED       41
#define COL_BG_GREEN     42
#define COL_BG_YELLOW    43
#define COL_BG_BLUE      44
#define COL_BG_MAGENTA   45
#define COL_BG_CYAN      46
#define COL_BG_WHITE     47

inline static void termhax_print_color(char* text, int color) {
    printf("\033[%dm%s\033[0m", color, text);
}

#endif // TERMHAX_H_