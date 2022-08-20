#ifndef __EBYTE_H__
#define __EBYTE_H__


#include "ebyte_e34.h"


extern void ebyte_setup();
extern void ebyte_process();  // Store & forward data between

extern Ebyte_E34 ebyte;
extern int ebyte_show_report_count;
extern bool ebyte_loopback_flag;


#endif  // __EBYTE_H__
