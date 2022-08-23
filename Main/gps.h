#ifndef __GPS_H__
#define __GPS_H__


#include <TinyGPS++.h>
#include <SoftwareSerial.h>

extern void gps_setup(bool do_axp_exist);
extern void gps_decoding_process();

extern int gps_print_count;


#endif  // __GPS_H__
