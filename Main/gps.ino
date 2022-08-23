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

static char str_date[20]    = {'\0'};
static char str_loc[32]     = {'\0'};
static char str_quality[10] = {'\0'};

#define GPS_PRINT_PERIOD 5000
int gps_print_count = 0;


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
    static uint32_t report_millis = millis() + GPS_PRINT_PERIOD;

    while (SERIAL_GPS.available()) {
        gps.encode(SERIAL_GPS.read());
    }

    if (gps_print_count != 0) {


        if (gps.satellites.isValid() && gps.time.isUpdated() && gps.location.isValid()) {

            if (millis() > report_millis) {
                // Example: http://arduiniana.org/libraries/tinygpsplus/
                gps_update_data();
                term_println( gps_update_str("[GPS] %s, (%s), Sat:%s") );

                if (gps_print_count > 0) {
                    gps_print_count--;
                }

                report_millis = millis() + GPS_PRINT_PERIOD;
            }
        }
    }
}

// ----------------------------------------------------------------------------
void gps_update_data() {
    snprintf(str_date, sizeof(str_date), "%02u-%02u-%04u %02u:%02u:%02u",
        gps.date.day(),  gps.date.month(),  gps.date.year(),
        gps.time.hour(), gps.time.minute(), gps.time.second());
    snprintf(str_loc, sizeof(str_loc), "%.6f, %.6f, %.2f",
        gps.location.lat(), gps.location.lng(), gps.altitude.meters());
    snprintf(str_quality, sizeof(str_quality), "%d",
        gps.satellites.value());
}

// ----------------------------------------------------------------------------
char *gps_update_str(const char *fmt) {
    if (str_date[0] == '\0' || str_loc[0] == '\0' || str_quality[0] == '\0') return NULL;

    static char str[sizeof(str_date) + sizeof(str_loc) + sizeof(str_quality) + 10];
    snprintf(str, sizeof(str), fmt, str_date, str_loc, str_quality);
    return str;
}
