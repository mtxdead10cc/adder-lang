#include "shared/sh_utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include <dlfcn.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

bool ut_file_path_exists(char *file_path) {
    if( file_path == NULL )
        return false;
    FILE *file = NULL;
    if ((file = fopen(file_path, "r"))) {
        fclose(file);
        return true;
    }
    return false;
}

time_t ut_file_path_get_modtime(char *file_path) {
    struct stat attr;
    stat(file_path, &attr);
    return attr.st_mtim.tv_sec;
}

size_t ut_file_get_size(ut_file_t handle) {
    if(handle == NULL)
        return 0;
    size_t restore = ftell((FILE*) handle);
    int error = fseek((FILE*) handle, 0, SEEK_END);
    assert(error == 0);
    size_t file_size = ftell((FILE*) handle);
    error = fseek((FILE*) handle, restore, SEEK_SET);
    assert(error == 0);
    return file_size;
}

size_t ut_file_read(ut_file_t handle, size_t start, char* buffer, size_t length) {

    if(handle == NULL)
        return 0;

    if(fseek((FILE*) handle, start, SEEK_SET) != 0)
        return 0;

    size_t size = ut_file_get_size((FILE*) handle);
    if(start > size)
        return 0;
    size = size - start;
    if(size > length)
        length = size;

    int retries = 100;
    while( retries > 0 ) {
        if( fread(buffer, length, 1, (FILE*) handle) > 0 ) {
            return length;
        } else {
            fseek((FILE*) handle, start, SEEK_SET);
            usleep(10000);
            retries --;
        }
    }

    return 0;
}

ut_file_t ut_file_open(char* file_path, char* mode) {
    if(file_path == NULL || mode == NULL)
        return NULL;
    return (void*) fopen(file_path, mode);
}

void ut_file_close(ut_file_t handle) {
    if(handle == NULL)
        return;
    int res = fclose((FILE*) handle);
    assert(res == 0);
}

