/**
 * @author Pasakorn Tiwatthanont
 *
 * Bug: arduino-vscode cannot find the header named with alphabet comming before the main .ino file.
 */
#include "global.h"


static bool do_axp_exist;  // T-Beam v0.7, early version, does't has AXP192 built-in.

// ---------- Setup ----------
void setup() {
    oled_setup();   // OLED

    do_axp_exist = axp_setup();  // Init axp20x and return T-Beam Version

    led_setup(do_axp_exist);    // LED
    gps_setup(do_axp_exist);    // GPS
    ebyte_setup();  // Ebyte

    term_printf("[MAIN] System initial success @ version: %s", __GIT_SHA1_ID__);
}

// ---------- Main ----------
void loop() {
    led_blinking_process();  // LED blinking
    gps_decoding_process();  // Process GPS data
    ebyte_process();  // Store & passing data between uC & Ebyte module

    if (do_axp_exist)
        axp_logging_process();  // Report energy usage on the node.
}
