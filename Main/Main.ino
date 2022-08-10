/**
 * @file Main.ino
 * @brief LoRa Relay Implementation on T-Beam ESP32 Platform with Motelib Flood Routing Support
 *
 * @author Pasakorn Tiwatthanont
 *
 * Serial as DEBUG & CLI
 * Serial1 as GPS
 * Serial2 as VTube connected with RTU
 * BT as data log
 *
 * Bug: arduino-vscode cannot find the header named with alphabet comming before the main .ino file.
 */
#include "global.h"

// #include <SPI.h>
// #include <LoRa.h>


static bool do_axp_exist;  // T-Beam v0.7, early version, does't has AXP192 installed.

// ---------- Setup ----------
void setup() {
    oled_setup();   // OLED

    do_axp_exist = axp_setup();  // Init axp20x and return T-Beam Version

    led_setup(do_axp_exist);    // LED
    gps_setup(do_axp_exist);    // GPS

    term_printf("[MAIN] System initial success @ version: %s", __GIT_SHA1_ID__);
}

// ---------- Main ----------
void loop() {
    led_blinking_process();  // LED blinking
    gps_decoding_process();  // Process GPS data

    if (do_axp_exist)
        axp_logging_process();  // Report energy usage on the node.
}
