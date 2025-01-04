#ifndef IFT_H_
#define IFT_H_

#include <sh_types.h>
#include <stdbool.h>
#include <stdint.h>

#define IFTYPE_MAX_TAGS 32

typedef enum ift_tag_t {
    IFT_UNK,
    IFT_VOID = 'v',
    IFT_BOOL = 'b',
    IFT_CHAR = 'c',
    IFT_I32 = 'i',
    IFT_F32 = 'f',
    IFT_LST = 'l',
    IFT_FUN = 'P',
    IFT_ENDFUN = 'p'
} ift_tag_t;

typedef struct ift_t {
    uint8_t count;
    uint8_t tags[IFTYPE_MAX_TAGS];
} ift_t;

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
int   ift_func_arg_count(ift_t func);
bool ift_type_equals(ift_t* a, ift_t* b);

sstr_t ift_type_to_sstr(ift_t type);
bool ift_type_equals(ift_t* a, ift_t* b);

#endif // IFT_H_