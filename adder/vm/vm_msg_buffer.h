#ifndef MSG_BUFFER_H_
#define MSG_BUFFER_H_

#include "sh_types.h"
#include "sh_utils.h"
#include <stdio.h>
#include <string.h>

inline static void vm_msg_buffer_init(vm_msg_buffer_t* buf, char* title) {
    buf->title = sstr(title);
    buf->count = 0;
    memset(buf->messages, 0, sizeof(buf->messages));
}

inline static void vm_msg_buffer_append(vm_msg_buffer_t* buf, sstr_t msg) {
    if( buf->count >= VM_MSG_BUFFER_MSG_MAX_COUNT )
        return;
    buf->messages[buf->count++] = msg;
}

inline static void vm_msg_buffer_fprint(vm_msg_buffer_t* buf, FILE* file) {
    fprintf(file, "Message buffer: ");
    sstr_fprint(file, &buf->title);

    for(int i = 0; i < buf->count; i++) {
        fprintf(file, "\n  ");
        sstr_fprint(file, &buf->messages[i]);
    }

    fprintf(file, "\n");
}

inline static void vm_msg_buffer_clear(vm_msg_buffer_t* buf) {
    buf->count = 0;
}

#endif // MSG_BUFFER_H_