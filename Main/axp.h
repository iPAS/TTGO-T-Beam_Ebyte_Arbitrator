#ifndef __AXP_H__
#define __AXP_H__


#include <axp20x.h>


extern bool axp_setup();
extern void axp_logging_process();      // Report energy usage on the node.

extern bool axp_exist;
extern int axp_show_report_count;


#endif  // __AXP_H__
