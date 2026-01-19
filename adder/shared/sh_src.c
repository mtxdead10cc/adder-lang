#include "shared/sh_src.h"
#include "shared/sh_utils.h"

src_t* src_create(char* name, char* buffer, size_t buffer_length) {
    size_t namelen = strlen(name);
    src_t* src = (src_t*) malloc(
        sizeof(src_t)
        + (namelen + 1)
        + (buffer_length + 1));

    if(src == NULL)
        return NULL;

    src->buff_length = buffer_length;
    src->path_length = namelen;
    src->path = src->dyndata;
    src->buff = src->dyndata + (namelen + 1);
    
    memcpy(src->path, name, namelen);
    memcpy(src->buff, buffer, buffer_length);

    src->path[namelen] = '\0';
    src->buff[buffer_length] = '\0';

    return src;
}

src_t* src_create_const(char* name, const char* buffer) {
    return src_create(name, (char*) buffer, strlen(buffer));
}

src_t* src_load(void* file_path) {

    ut_file_t file = ut_file_open(file_path, "r");

    if(file == NULL)
        return NULL;
    
    size_t pathlen = strlen(file_path);
    size_t datalen = ut_file_get_size(file);

    src_t* src = (src_t*) malloc(
        sizeof(src_t)
        + (pathlen + 1)
        + (datalen + 1));

    if(src == NULL) {
        ut_file_close(file);
        return NULL;
    }

    src->path_length = pathlen;
    src->buff_length = datalen;

    src->path = src->dyndata;
    src->buff = src->dyndata + pathlen + 1;
    
    memcpy(src->path, file_path, pathlen);
    
    size_t readlen = ut_file_read(
        file, 0,
        src->buff,
        datalen);

    ut_file_close(file);
    
    if(readlen != datalen) {
        free(src);
        return NULL;
    }

    src->path[pathlen] = '\0';
    src->buff[datalen] = '\0';

    return src;
}

srcref_t src_linsearch(src_t* src, const char* pattern) {
    size_t len = strlen(pattern);
    if( len == 0 )
        return srcref_full_range(src);
    if( src->buff_length < len )
        return (srcref_t) {0};
    // TODO: Not sure about this!
    size_t stop = src->buff_length - len + 1;
    for(size_t start = 0; start < stop; start++) {
        if(src->buff[start] != pattern[0])
            continue;
        if(strncmp(src->buff + start, pattern, len) != 0)
            continue;
        return srcref(src, start, len);
    }
    return (srcref_t) {0};
}

void src_destroy(src_t* src) {
    if( src == NULL )
        return;
    size_t clear_size = sizeof(src_t)
        + (src->path_length + 1)
        + (src->buff_length + 1);
    memset(src, 0, clear_size);
    free(src);
}