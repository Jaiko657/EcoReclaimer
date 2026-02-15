#include "engine/core/logger/logger_backend.h"
#include "engine/core/logger/logger.h"

#include <stdio.h>
#include <stdarg.h>

static void stdout_sink(log_level_t lvl, const log_cat_t *cat, const char *fmt, va_list ap)
{
    static const char *N[] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };
    (void)lvl;
    fprintf(stdout, "[%s]%s%s%s", N[lvl],
            cat && cat->name ? "[" : "",
            cat && cat->name ? cat->name : "",
            cat && cat->name ? "] " : " ");
    vfprintf(stdout, fmt, ap);
    fputc('\n', stdout);
    fflush(stdout);
}

void logger_backend_init(void)
{
    log_set_sink(stdout_sink);
}
