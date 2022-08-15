#include "global.h"


/******************************************************************************
 * For debugging
 */
void term_printf(const char *format, ...)
{
    static char buf[SIZE_DEBUG_BUF];
    char *p = buf;
    va_list ap;
    va_start(ap, format);
    vsnprintf(p, sizeof(buf), format, ap);

    term_println(buf);

    va_end(ap);
}
