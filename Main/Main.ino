/**
 * @author Pasakorn Tiwatthanont
 *
 * Bug: arduino-vscode cannot find the header named with alphabet comming before the main .ino file.
 */
#include "global.h"


static bool do_axp_exist;  // T-Beam v0.7, early version, does't has AXP192 built-in.

// ---------- Setup ----------
void setup() {
    vTaskDelay(1500 / portTICK_PERIOD_MS);  // Wait debugging console

    do_axp_exist = axp_setup(); // Init axp20x and return T-Beam Version
    led_setup(do_axp_exist);    // LED
    cli_setup();
    ebyte_setup();  // Ebyte

    term_printf(ENDL "[MAIN] System initialized successfully @rev: %s" ENDL, __GIT_SHA1_ID__);
}

// ---------- Main ----------
void loop() {
    if (do_axp_exist)
        axp_logging_process();  // Report energy usage on the node.
    led_blinking_process();  // LED blinking
    cli_interpretation_process();  // Interpret command-line

    ebyte_process();  // Store & passing data between uC & Ebyte module

    taskYIELD();
}
