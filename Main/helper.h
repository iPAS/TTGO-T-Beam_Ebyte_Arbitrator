#ifndef __HELPER_H__
#define __HELPER_H__


#define SIZE_DEBUG_BUF 255
#define ENDL "\n\r"

#define term_print(...)   { Serial.print(__VA_ARGS__); }
#define term_println(...) { Serial.println(__VA_ARGS__); }

extern void term_printf(const char *format, ...);

#define debug(args...) term_printf("[X] " args)
#ifndef debug
#define debug(args...)
#endif

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))


extern String hex_stream(const void * p, uint16_t len);
extern boolean is_numeric(String str);
extern bool extract_int(String str, long *ret);


#endif  // __HELPER_H__
