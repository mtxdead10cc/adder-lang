#ifndef GVM_ASM_H_
#define GVM_ASM_H_

#include "gvm_types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*

EXAMPLE

label:
    push 5
    push 0
    loop:
        dup 2
        is-less
        if-false
            jump exit-loop
        push 1
        add
        jump loop
    exit-loop:
        quit

TODO

    Handle conversion from gvmasm to byte code
    [ ] parse the asm code
    [ ] read up the constants and put them in the data section
    [ ] replace constants with references to the data section
    [ ] replace labels with actual adresses
    [ ] encode data into val_t format


LIST

    1. collect all label locations and names, remove line
    2. collect all constant entries

*/

gvm_result_t asm_assemble(char* code_buffer);

#endif // GVM_ASM_H_