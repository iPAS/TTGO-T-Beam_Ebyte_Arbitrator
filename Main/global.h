#ifndef __GLOBAL_H__
#define __GLOBAL_H__


#include <Arduino.h>
#include <strings.h>
#include "version.h"
#include "helper.h"


extern void oled_setup();

extern bool axp_setup();
extern void axp_logging_process();  // Report energy usage on the node.

extern void led_setup(bool do_axp_exist);
extern void led_blinking_process();  // LED blinking

extern void gps_setup(bool do_axp_exist);
extern void gps_decoding_process();  // Process GPS data


#endif  // __GLOBAL_H__
