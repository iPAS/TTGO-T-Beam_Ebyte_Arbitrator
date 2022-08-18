#include "global.h"


/**
 * @brief Print to terminal
 */
void term_printf(const char *format, ...)
{
    static char buf[SIZE_DEBUG_BUF];
    char *p = buf;
    va_list ap;
    va_start(ap, format);
    vsnprintf(p, sizeof(buf), format, ap);

    term_print(buf);

    va_end(ap);
}

/**
 * @brief Check whether numeric or not
 */
boolean is_numeric(String str) {
    // http://tripsintech.com/arduino-isnumeric-function/
    unsigned int stringLength = str.length();

    if (stringLength == 0) {
        return false;
    }

    boolean seenDecimal = false;

    for(unsigned int i = 0; i < stringLength; ++i) {
        if (isDigit(str.charAt(i))) {
            continue;
        }

        if (str.charAt(i) == '.') {
            if (seenDecimal) {
                return false;
            }
            seenDecimal = true;
            continue;
        }
        return false;
    }
    return true;
}

/**
 * @brief Extract number from String with checking
 */
bool extract_int(String str, int *ret) {
    long id = str.toInt();

    if (id == 0) {
        if (is_numeric(str)) {  // Re-check if being zero
            *ret = id;
            return true;
        }
        return false;
    }

    return true;
}
