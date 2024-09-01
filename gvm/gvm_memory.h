#ifndef GVM_MENORY_H_
#define GVM_MEMORY_H_

#include <stdint.h>
#include <stdbool.h>
#include "gvm_types.h"

#define MEM_IS_CONST_ADDR(VAL_ADDR) (((VAL_ADDR) >> 15) == 0)
#define MEM_MK_CONST_ADDR(INDEX)    (((val_addr_t)(INDEX)) & 0x7FFF)
#define MEM_MK_PROGR_ADDR(INDEX)    (((val_addr_t)(INDEX)) | 0x8000)
#define MEM_ADDR_TO_INDEX(VAL_ADDR) (int)((VAL_ADDR) & 0x7FFF)

#endif // GVM_MEMORY_H_