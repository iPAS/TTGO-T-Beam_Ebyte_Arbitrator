#ifndef __EBYTE_H__
#define __EBYTE_H__


#include "ebyte_e34.h"


extern void ebyte_setup();
extern void ebyte_process();  // Store & forward data between
extern void ebyte_set_airrate(uint8_t level);

extern Ebyte_E34 ebyte;
extern int ebyte_show_report_count;
extern bool ebyte_loopback_flag;
extern uint8_t ebyte_airrate_level;
extern uint8_t ebyte_txpower_level;


#endif  // __EBYTE_H__
