#include "global.h"


#define AXP_REPORT_PERIOD (60000 * 1)

#define AXP_SDA 21
#define AXP_SCL 22
#define AXP_IRQ 35

static AXP20X_Class axp;
static uint32_t axp_report_millis;
bool axp_exist = false;
int axp_show_report_count = 0;  // 0 is 'disable', -1 is 'forever', other +n will be counted down to zero.


static char str_axp_temp[10] = {'\0'};
static char str_axp_bus[20] = {'\0'};
static char str_axp_bat[60] = {'\0'};

// ----------------------------------------------------------------------------
bool axp_setup() {
    Wire.begin(AXP_SDA, AXP_SCL);
    if (axp.begin(Wire, AXP192_SLAVE_ADDRESS) == AXP_FAIL) {
        term_println("[AXP] Starting AXP192 failed! -- guessing, this is the V0.7");
        return false;
    }

    axp_exist = true;
    term_println("[AXP] Starting AXP192 succeeded! -- guessing, its version >= V1.0");

    /*
    DCDC1 0.7-3.5V @ 1200mA max -> OLED // If you turn this off you'll lose comms to the axp192 because the
                                        // OLED and the axp192 share the same i2c bus, instead use ssd1306 sleep mode
    DCDC2 -> unused
    DCDC3 0.7-3.5V @ 700mA max -> ESP32 (keep this on!)
    LDO1 30mA -> charges GPS backup battery // Charges the tiny J13 battery by the GPS
                                            //  to power the GPS ram (for a couple of days), can not be turned off
    LDO2 200mA -> LORA
    LDO3 200mA -> GPS
    */

    axp.setBackupChargeControl(true);  // LDO1 for GPS battery backup, cannot turn off.

    axp.setLDO2Voltage(3300);
    axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);  // LORA radio

    axp.setLDO3Voltage(3300);
    axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);  // GPS main power

    axp.setDCDC1Voltage(3300);
    axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);  // for the OLED SSD1306 power, cannot be off, shared with AXP

    axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);  // Unused actually

    axp.setPowerOutPut(AXP192_DCDC3, AXP202_ON);  // ESP32 -- off this, die!

    axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);  // External power

    axp.enableChargeing(true);  // Enable by default actually

    // axp.setChgLEDMode(AXP20X_LED_OFF);  // LED off
    // axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);

    // pinMode(PMU_IRQ, INPUT_PULLUP);
    // attachInterrupt(PMU_IRQ, [] { pmu_irq = true; }, FALLING);
    // axp.enableIRQ(AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_BATT_REMOVED_IRQ | AXP202_BATT_CONNECT_IRQ, 1);
    // axp.clearIRQ();

    axp.adc1Enable(
        AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1,
        true);

    axp.ClearCoulombcounter();
    axp.EnableCoulombcounter();

    axp_report_millis = millis();

    return true;
}

// ----------------------------------------------------------------------------
void axp_logging_process() {
    if (millis() > axp_report_millis) {
        if (axp_show_report_count > 0  ||  axp_show_report_count < 0) {
            axp_update_data();
            if (str_axp_temp[0] != '\0' 
            &&  str_axp_bus[0]  != '\0' 
            &&  str_axp_bat[0]  != '\0') {
                term_printf("[AXP] %s, BUS(%s), BAT(%s)" ENDL, str_axp_temp, str_axp_bus, str_axp_bat);
            }

            if (axp_show_report_count > 0)
                axp_show_report_count--;
        }
        axp_report_millis = millis() + AXP_REPORT_PERIOD;
    }
}

// ----------------------------------------------------------------------------
void axp_update_data() {
    snprintf(str_axp_temp, sizeof(str_axp_temp),
        "%.2f℃", axp.getTemp());

    snprintf(str_axp_bus, sizeof(str_axp_bus),
        "%.3fV, %.2fmA", axp.getVbusVoltage() / 1000, axp.getVbusCurrent());

    snprintf(str_axp_bat, sizeof(str_axp_bat),
        "%.3fmW, %.3fV, %.2fmA, %.2fmA, %.3fmAh",
        axp.getBattInpower(), axp.getBattVoltage() / 1000,
        axp.getBattChargeCurrent(), axp.getBattDischargeCurrent(), axp.getCoulombData());
}
