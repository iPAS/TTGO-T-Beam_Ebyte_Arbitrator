#include "global.h"


#define EBYTE_SERIAL    Serial2
#define EBYTE_BAUD      UART_BPS_RATE_9600
#define EBYTE_PIN_TX    2
#define EBYTE_PIN_RX    13
#define EBYTE_PIN_AUX   34
#define EBYTE_PIN_M0    25
#define EBYTE_PIN_M1    14
#define EBYTE_UART_BUFFER_SIZE 512
#define EBYTE_UART_BUFFER_TMO 1000

static LoRa_E32 ebyte(&EBYTE_SERIAL, EBYTE_PIN_AUX, EBYTE_PIN_M0, EBYTE_PIN_M1, EBYTE_BAUD);

#define EBYTE_PERIOD    600000
static uint32_t ebyte_period_millis = 0;


// ----------------------------------------------------------------------------
void ebyte_setup() {
    EBYTE_SERIAL.setRxBufferSize(EBYTE_UART_BUFFER_SIZE);
    EBYTE_SERIAL.setTimeout(EBYTE_UART_BUFFER_TMO);
    EBYTE_SERIAL.begin(EBYTE_BAUD, SERIAL_8N1, EBYTE_PIN_RX, EBYTE_PIN_TX);
    while (!EBYTE_SERIAL)
        vTaskDelay(1);  // Yield
    while (EBYTE_SERIAL.available())
        EBYTE_SERIAL.read();  // Clear buffer

    ebyte.begin();  // Start communication with Ebyte module: config & etc.
}

// ----------------------------------------------------------------------------
void ebyte_process() {
    if (millis() > ebyte_period_millis) {
        ResponseStructContainer c;
        c = ebyte.getConfiguration();
        Configuration configuration = *((Configuration *)c.data);  // It's important get configuration pointer before all other operation
        term_println(c.status.getResponseDescription());  // Description of code
        term_println(c.status.code);  // 1 if Success
        // printParameters(configuration);

        c.close();
        ebyte_period_millis = millis() + EBYTE_PERIOD;  // Reset timeout
        term_println("\n ++++++++++++++++++++++++++++++++++++++++++++ \n");
    }
}

// void printParameters(struct Configuration configuration) {
// 	term_println("----------------------------------------");

// 	term_print(F("HEAD BIN: "));  term_print(configuration.HEAD, BIN);term_print(" ");term_print(configuration.HEAD, DEC);term_print(" ");term_println(configuration.HEAD, HEX);
// 	term_println(F(" "));
// 	term_print(F("AddH BIN: "));  term_println(configuration.ADDH, BIN);
// 	term_print(F("AddL BIN: "));  term_println(configuration.ADDL, BIN);
// 	term_print(F("Chan BIN: "));  term_print(configuration.CHAN, DEC); term_print(" -> "); term_println(configuration.getChannelDescription());
// 	term_println(F(" "));
// 	term_print(F("SpeedParityBit BIN    : "));  term_print(configuration.SPED.uartParity, BIN);term_print(" -> "); term_println(configuration.SPED.getUARTParityDescription());
// 	term_print(F("SpeedUARTDataRate BIN : "));  term_print(configuration.SPED.uartBaudRate, BIN);term_print(" -> "); term_println(configuration.SPED.getUARTBaudRate());
// 	term_print(F("SpeedAirDataRate BIN  : "));  term_print(configuration.SPED.airDataRate, BIN);term_print(" -> "); term_println(configuration.SPED.getAirDataRate());

// 	term_print(F("OptionTrans BIN       : "));  term_print(configuration.OPTION.fixedTransmission, BIN);term_print(" -> "); term_println(configuration.OPTION.getFixedTransmissionDescription());
// 	term_print(F("OptionPullup BIN      : "));  term_print(configuration.OPTION.ioDriveMode, BIN);term_print(" -> "); term_println(configuration.OPTION.getIODroveModeDescription());
// 	term_print(F("OptionWakeup BIN      : "));  term_print(configuration.OPTION.wirelessWakeupTime, BIN);term_print(" -> "); term_println(configuration.OPTION.getWirelessWakeUPTimeDescription());
// 	term_print(F("OptionFEC BIN         : "));  term_print(configuration.OPTION.fec, BIN);term_print(" -> "); term_println(configuration.OPTION.getFECDescription());
// 	term_print(F("OptionPower BIN       : "));  term_print(configuration.OPTION.transmissionPower, BIN);term_print(" -> "); term_println(configuration.OPTION.getTransmissionPowerDescription());

// 	term_println("----------------------------------------");

// }

// void printModuleInformation(struct ModuleInformation moduleInformation) {
// 	term_println("----------------------------------------");
// 	term_print(F("HEAD BIN: "));  term_print(moduleInformation.HEAD, BIN);term_print(" ");term_print(moduleInformation.HEAD, DEC);term_print(" ");term_println(moduleInformation.HEAD, HEX);

// 	term_print(F("Freq.: "));  term_println(moduleInformation.frequency, HEX);
// 	term_print(F("Version  : "));  term_println(moduleInformation.version, HEX);
// 	term_print(F("Features : "));  term_println(moduleInformation.features, HEX);
// 	term_println("----------------------------------------");
// }
