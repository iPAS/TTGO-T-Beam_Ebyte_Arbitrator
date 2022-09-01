#ifndef __MAIN_H__
#define __MAIN_H__


typedef enum {
    VERBOSE_NONE    = 0,
    VERBOSE_ERROR   = 1,
    VERBOSE_WARNING = 2,
    VERBOSE_INFO    = 3,
    VERBOSE_DEBUG   = 4,
} verbose_level_t;

extern verbose_level_t system_verbose_level;


#endif  // __MAIN_H__
