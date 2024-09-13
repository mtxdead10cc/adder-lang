#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include "termhax.h"

typedef enum keypress_t {
    KEY_W,
    KEY_S,
    KEY_D,
    KEY_A,
    KEY_SPACE,
    KEY_Q,
    KEY_PLUS,
    KEY_MINUS,
    KEY_ARROW_UP,
    KEY_ARROW_DOWN,
    KEY_ARROW_LEFT,
    KEY_ARROW_RIGHT,
    KEY_ESCAPE,
    KEY_UNK
} keypress_t;

inline static int kb_getch() { 
    int ch;
    struct termios oldattr;
    struct termios newattr;
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

inline static int char_to_upper(int c) {
    if( c >= 'a' && c <= 'z' ) {
        int offs = c - 'a';
        return 'A' + offs;
    } else {
        return c;
    }
}

inline static keypress_t _kb_read(bool escaped, bool arrows) {
    if( escaped ) {
        if( arrows == false ) { // escaped
            switch(kb_getch()) {
                case '\033': return KEY_ESCAPE; // escape again
                case  0x5B : return _kb_read(true, true);
                default: return KEY_UNK;
            }
        } else { // arrow keys
            switch(kb_getch()) {
                case 0x43: return KEY_ARROW_RIGHT;
                case 0x44: return KEY_ARROW_LEFT;
                case 0x41: return KEY_ARROW_UP;
                case 0x42: return KEY_ARROW_DOWN;
                default: return KEY_UNK;
            }
        }
    } else {
        switch(char_to_upper(kb_getch())) {
            case 'A': return KEY_A;
            case 'W': return KEY_W;
            case 'S': return KEY_S;
            case 'D': return KEY_D;
            case 'Q': return KEY_Q;
            case '+': return KEY_PLUS;
            case '-': return KEY_MINUS;
            case ' ': return KEY_SPACE;
            case '\033': return _kb_read(true, false);
            default: return KEY_UNK;
        }
    }
}

#define kb_read() _kb_read(false, false)

#endif // KEYBOARD_H_