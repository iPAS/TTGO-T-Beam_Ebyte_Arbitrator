#include "global.h"


#define SERIAL_GPS ss
#define GPS_BAUDRATE 9600
#define GPS_TX_V07 15
#define GPS_RX_V07 12
#define GPS_TX_V10 12
#define GPS_RX_V10 34

SoftwareSerial SERIAL_GPS;
static uint8_t gps_tx;
static uint8_t gps_rx;

static TinyGPSPlus gps;

static char str_gps_datetime[20] = {'\0'};
static char str_gps_loc[32] = {'\0'};
static char str_gps_quality[10] = {'\0'};


// ----------------------------------------------------------------------------
void gps_setup(bool do_axp_exist) {
    if (do_axp_exist) {
        gps_tx = GPS_TX_V10;
        gps_rx = GPS_RX_V10;
    }
    else {
        gps_tx = GPS_TX_V07;
        gps_rx = GPS_RX_V07;
    }

    SERIAL_GPS.begin(GPS_BAUDRATE, SWSERIAL_8N1, gps_rx, gps_tx);
    while (!SERIAL_GPS)
        vTaskDelay(1);  // Yield
    while (SERIAL_GPS.available())
        SERIAL_GPS.read();  // Clear buffer
}

// ----------------------------------------------------------------------------
void gps_decoding_process() {
    // while (SERIAL_GPS.available()) {
    //     gps.encode(SERIAL_GPS.read());
    // }

    // // --------------------
    // // Print time cyclingly
    // // --------------------
    // if (millis() > next_gps_stamp_millis) {
    //     if (gps.satellites.isValid() && gps.time.isUpdated() && gps.location.isValid()) {
    //         // Example: http://arduiniana.org/libraries/tinygpsplus/
    //         gps_update_data();
    //         term_println( gps_update_str("[GPS] %s, (%s), Sat:%s") );
    //     }
    //     next_gps_stamp_millis = millis() + GPS_STAMP_PERIOD;

    //     if (getAddress() != SINK_ADDRESS) {
    //         if (millis() > next_gps_report_millis) {
    //             if (report_gps_to(SINK_ADDRESS) == false) {
    //                 term_println("[GPS] Reporting failed!");
    //             }
    //             next_gps_report_millis = millis() + GPS_REPORT_PERIOD;
    //         }
    //     }
    // }
}

// ----------------------------------------------------------------------------
void gps_update_data() {
    snprintf(str_gps_datetime, sizeof(str_gps_datetime), "%02u-%02u-%04u %02u:%02u:%02u",
        gps.date.day(),  gps.date.month(),  gps.date.year(),
        gps.time.hour(), gps.time.minute(), gps.time.second());
    snprintf(str_gps_loc, sizeof(str_gps_loc), "%.6f, %.6f, %.2f",
        gps.location.lat(), gps.location.lng(), gps.altitude.meters());
    snprintf(str_gps_quality, sizeof(str_gps_quality), "%d",
        gps.satellites.value());
}

// ----------------------------------------------------------------------------
char *gps_update_str(const char *fmt) {
    if (str_gps_datetime[0] == '\0' || str_gps_loc[0] == '\0' || str_gps_quality[0] == '\0') return NULL;

    static char str[sizeof(str_gps_datetime) + sizeof(str_gps_loc) + sizeof(str_gps_quality) + 10];
    snprintf(str, sizeof(str), fmt, str_gps_datetime, str_gps_loc, str_gps_quality);
    return str;
}
