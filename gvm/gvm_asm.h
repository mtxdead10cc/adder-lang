#ifndef GVM_ASM_H_
#define GVM_ASM_H_

#include "gvm_types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define READ_I16(D, AT) ((int16_t)((D)[(AT) + 1] << 8) | (int16_t) (D)[(AT)])

code_object_t asm_assemble_code_object(char* code_buffer);
void asm_debug_disassemble_code_object(code_object_t* code_object);
void asm_destroy_code_object(code_object_t* code_object);

#endif // GVM_ASM_H_