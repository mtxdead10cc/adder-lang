#include "sh_log.h"
#include "sh_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct sh_log_data_t {
    sh_logprintfn_t print;
} sh_log_data_t;

static sh_log_data_t logobj = { .print = NULL };

const char* sh_log_preabmle(sh_log_tag_t tag) {
    switch(tag) {
        case SH_LOG_DEFAULT: return "";
        case SH_LOG_ERROR: return "\033[31mERROR\033[0m: "; // RED
        case SH_LOG_WARNING: return "\033[33mWARNING\033[0m: "; // YELLOW
        case SH_LOG_INFO: return "\033[32mINFO\033[0m: "; // GREEN
        default: return "\033[44mOTHER\033[0m: "; // BG BLUE
    }
}

void sh_log_init(sh_logprintfn_t printfn) {
    logobj.print = printfn;
}

void print_wrapper(sh_log_tag_t tag, char* fmt, va_list args) {
    const char* pre = sh_log_preabmle(tag);
    int pre_len = strnlen(pre, 32);
    int fmt_len = strnlen(fmt, 2048);
    int len = pre_len + fmt_len;
    
    char buf[len + 2];
    strncpy(buf, pre, pre_len);
    strncpy(buf + pre_len, fmt, fmt_len);

    int last = len - 1;
    if( len == 0 )
        return;
    
    while (last > 0) {
        if( buf[last] != '\n' )
            break;
        last --;
    }
    buf[++last] = '\n';
    buf[++last] = '\0';

    if( logobj.print != NULL )
        logobj.print(tag, buf, args);
    else
        vprintf(buf, args);
}

void _sh_log_message(sh_log_tag_t tag, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    print_wrapper(tag, fmt, args);
    va_end(args);
}