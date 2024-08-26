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
    [X] parse the asm code
    [X] read up the constants and put them in the data section
    [X] replace constants with references to the data section
    [X] replace labels with actual adresses
    [X] encode data into val_t format
    [ ] bytcode const refs are always u8 now, change to u16?

*/

code_object_t asm_assemble_code_object(char* code_buffer);
void asm_debug_disassemble_code_object(code_object_t* code_object);
void asm_destroy_code_object(code_object_t* code_object);

#endif // GVM_ASM_H_