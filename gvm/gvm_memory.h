#ifndef GVM_MENORY_H_
#define GVM_MEMORY_H_

#include <stdint.h>
#include <stdbool.h>
#include "gvm_types.h"

#define MEM_CONST_FLAG 1
#define MEM_PROGR_FLAG 2

#define MEM_MAX_ADDRESSABLE 0x3FFFFFFF
#define ADDR_NIL 0U

#define ADDR_IS_CONST(VAL_ADDR) (((VAL_ADDR) >> 30) == MEM_CONST_FLAG)
#define ADDR_IS_PROGR(VAL_ADDR) (((VAL_ADDR) >> 30) == MEM_PROGR_FLAG)
#define ADDR_IS_NULL(VAL_ADDR)  (((VAL_ADDR) >> 30) == 0)

#define MEM_MK_CONST_ADDR(INDEX)    ((val_addr_t)( 0x40000000 | ((INDEX) & 0x3FFFFFFF)))
#define MEM_MK_PROGR_ADDR(INDEX)    ((val_addr_t)( 0x80000000 | ((INDEX) & 0x3FFFFFFF)))

#define MEM_ADDR_TO_INDEX(VAL_ADDR) (uint32_t)((VAL_ADDR) & 0x3FFFFFFF)


#endif // GVM_MEMORY_H_