#include <stdio.h>
#include <gvm.h>

int main(int argv, char** argc) {
    printf("INPUTS\n");
    for(int i = 0; i < argv; i++) {
        printf("  %i: %s\n", i, argc[i]);
    }
    test();
    return 0;
}