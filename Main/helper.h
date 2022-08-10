#ifndef __HELPER_H__
#define __HELPER_H__


#include "global.h"


#define SIZE_DEBUG_BUF 255

#define term_print(arg)   { Serial.print(arg); }
#define term_println(arg) { Serial.println(arg); }
extern void term_printf(const char *format, ...);

#define debug(args...) term_printf("[X] " args)
#ifndef debug
#define debug(args...)
#endif


#endif  // __HELPER_H__
