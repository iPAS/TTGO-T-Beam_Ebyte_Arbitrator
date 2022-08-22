#include "global.h"


verbose_code_t system_verbose_level = VERBOSE_NONE;


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
 * @brief HEX stream
 */
String hex_stream(const void * p, uint16_t len) {
    String str;
    while (len--) {
        // term_printf(" %2X", *p++);
        str += String(*((char *)p++), HEX);
        str += " ";
    }
    return str;
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
bool extract_int(String str, long *ret) {
    long value = str.toInt();

    if (value == 0) {
        if (is_numeric(str) == false)  // Re-check if being zero
            return false;
    }

    *ret = value;
    return true;
}
