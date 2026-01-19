#ifndef VM_UTILS_H_
#define VM_UTILS_H_

#include "shared/sh_types.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>

#define READ_U32(D, AT) ((uint32_t)((0xFF & (D)[(AT) + 3]) << (8*3)) |\
                         (uint32_t)((0xFF & (D)[(AT) + 2]) << (8*2)) |\
                         (uint32_t)((0xFF & (D)[(AT) + 1]) << (8*1)) |\
                         (uint32_t)((0xFF & (D)[(AT) + 0]) << (8*0)))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define clamp(v, a, b) (max(min((v), b), a))

typedef void* ut_file_t;

time_t      ut_file_path_get_modtime(char *file_path);
bool        ut_file_path_exists(char *file_path);
ut_file_t   ut_file_open(char* file_path, char* mode);
void        ut_file_close(ut_file_t handle);
size_t      ut_file_read(ut_file_t handle, size_t start, char* buffer, size_t length);
size_t      ut_file_get_size(ut_file_t handle);

#endif // VM_UTILS_H_
