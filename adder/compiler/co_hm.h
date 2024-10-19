#ifndef HM_H_
#define HM_H_

/* Hinly-Milner
    Guide
    https://bernsteinbear.com/blog/type-inference/
*/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#include "co_types.h"
#include "sh_arena.h"
#include "co_utils.h"

/*
 variable   : [variable, <name>]
 value  
  - int     : [int]
  - float   : [float]
  - bool    : [bool]
  - char    : [char]
 list       : [list]
 function   : [function, variable, <arg-name>, <type>]
*/

typedef enum tyexp_kind_t {
    TYEXP_VAR,
    TYEXP_INT,
    TYEXP_FLOAT,
    TYEXP_BOOL,
    TYEXP_CHAR,
    TYEXP_LIST,
    TYEXP_FUNCTION,
    TYEXP_APPLY,
    TYEXP_DEFINE
} tyexp_kind_t;

typedef struct tyexp_t tyexp_t;
typedef struct tyexp_t {
    tyexp_kind_t kind;
    union  {
        struct {
            sstr_t name;
        } var;
        struct {
            tyexp_t* arg;
            tyexp_t* body;
        } fun;
        struct {
            tyexp_t* func;
            tyexp_t* arg;
        } app;
        struct {
            tyexp_t* bind_name;
            tyexp_t* bind_value;
            tyexp_t* body;
        } def;
    };
} tyexpr_t;

typedef enum hm_tag_t {
    HM_TYPE,
    HM_VAR,
    HM_CON,
    HM_FORALL
} hm_tag_t;

typedef struct hm_type_t hm_type_t;

typedef struct hm_var_t {
    sstr_t      name;
    hm_type_t*  next; // union find
} hm_var_t;

typedef struct hm_constant_t {
    sstr_t      name;
    struct {
        size_t count;
        hm_type_t* array[2];
    } args;
} hm_constant_t;

typedef struct hm_forall_t {
    struct {
        size_t count;
        hm_var_t* array;
    } type_vars;
    hm_type_t* type;
} hm_forall_t;

typedef struct hm_type_t {
    hm_tag_t tag;
    union {
        hm_var_t       var;
        hm_constant_t  con;
        hm_forall_t    forall;
    } u;
} hm_type_t;


typedef struct hm_map_t {
    sstr_t*     keyptrs;
    hm_type_t*  valptrs;
    size_t      count;
    size_t      capacity;
} hm_map_t;

hm_map_t* hm_map_create();
void hm_map_destroy(hm_map_t* m);
hm_map_t* hm_map_copy(hm_map_t* other);
hm_type_t* hm_map_get(hm_map_t* m, sstr_t* name);
void hm_map_delete(hm_map_t* m, sstr_t* name);
void hm_map_set(hm_map_t* m, sstr_t* name, hm_type_t* type);


#endif // HM_H_