#ifndef GVM_H_
#define GVM_H_

#include "gvm_types.h"

void test();

/*

    TODO
        Implement the VM execution engine
        [ ] Duplicate
        [ ] SwapTop
        [ ] Load
        [ ] Goto
        [ ] Jump
        [ ] JumpIfFalse
        [ ] CompareEqual
        [ ] CompareLessThan
        [ ] CompareMoreThan
        [ ] And
        [ ] Or
        [ ] Nor
        [ ] Not
        [ ] Negate
        [ ] Mutiply
        [ ] Add
        [ ] Subtract
*/

char* gvm_result_to_string(gvm_result_t res);
void gvm_print_if_error(gvm_result_t res, char* context);


#endif // GVM_H_