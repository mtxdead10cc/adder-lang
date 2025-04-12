#ifndef SH_LOG_H_
#define SH_LOG_H_

#include <stdarg.h>
#include "sh_types.h"

typedef enum sh_log_tag_t {
    SH_LOG_DEFAULT,
    SH_LOG_ERROR,
    SH_LOG_WARNING,
    SH_LOG_INFO,
} sh_log_tag_t;

typedef void (*sh_logprintfn_t)(sh_log_tag_t tag, const char *fmt, va_list args);

const char* sh_log_preabmle(sh_log_tag_t type);
void sh_log_init(sh_logprintfn_t printfn);
void _sh_log_message(sh_log_tag_t tag, char* fmt, ...);

#define sh_log_error(...) _sh_log_message(SH_LOG_ERROR, __VA_ARGS__)
#define sh_log_warning(...) _sh_log_message(SH_LOG_WARNING, __VA_ARGS__)
#define sh_log_info(...) _sh_log_message(SH_LOG_INFO, __VA_ARGS__)
#define sh_log(...) _sh_log_message(SH_LOG_DEFAULT, __VA_ARGS__)

#define SH_LOG_MAX_MESSAGE_LENGTH (2048 * 5)

#endif // SH_LOG_H_