#ifndef IFT_H_
#define IFT_H_

#include <sh_types.h>
#include <stdbool.h>
#include <stdint.h>



ift_t ift_void(void);
ift_t ift_int(void);
ift_t ift_float(void);
ift_t ift_char(void);
ift_t ift_bool(void);
ift_t ift_unknown(void);

ift_t ift_list(ift_t content_type);
ift_t ift_list_get_content_type(ift_t list);
ift_t ift_func(ift_t return_type);
ift_t ift_func_add_arg(ift_t func, ift_t arg);
ift_t ift_func_get_arg(ift_t func, int index);
ift_t ift_func_get_return_type(ift_t func);

ift_t ift_func_1(ift_t return_type, ift_t arg1);
ift_t ift_func_2(ift_t return_type, ift_t arg1, ift_t arg2);
ift_t ift_func_3(ift_t return_type, ift_t arg1, ift_t arg2, ift_t arg3);

int   ift_func_arg_count(ift_t func);
bool ift_type_equals(ift_t* a, ift_t* b);

sstr_t ift_type_to_sstr(ift_t type);
bool ift_type_equals(ift_t* a, ift_t* b);
bool ift_is_unknown(ift_t type);

#endif // IFT_H_