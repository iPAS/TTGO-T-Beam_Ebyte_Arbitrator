#include "global.h"


#define EBYTE_SERIAL    Serial2
#define EBYTE_BAUD      115200
#define EBYTE_PIN_E34_RX    2   // uC TX
#define EBYTE_PIN_E34_TX    13  // uC RX
#define EBYTE_PIN_AUX   34
#define EBYTE_PIN_M0    25
#define EBYTE_PIN_M1    14
#define EBYTE_UART_BUFFER_SIZE 512
#define EBYTE_UART_BUFFER_TMO 1000

static Ebyte_E34 ebyte(&EBYTE_SERIAL, EBYTE_PIN_AUX, EBYTE_PIN_M0, EBYTE_PIN_M1, EBYTE_PIN_E34_RX, EBYTE_PIN_E34_TX);

#define EBYTE_PERIOD 600000
static uint32_t ebyte_period_millis = 0;

// ----------------------------------------------------------------------------
void ebyte_setup() {
    if (ebyte.begin()) {  // Start communication with Ebyte module: config & etc.
        term_println("\n[EBYTE] initialized successfully");

        ResponseStructContainer c;
        c = ebyte.getConfiguration();  // Get c.data from here
        Configuration cfg = *((Configuration *)c.data); // This is a memory transfer, NOT by-reference.
                                                        // It's important get configuration pointer before all other operation.
        c.close();  // Clean c.data that was allocated in ::getConfiguration()

        if (c.status.code == E34_SUCCESS){
            ebyte.printParameters(&cfg);

            // Setup the desired mode
            cfg.ADDH = 0;
            cfg.ADDL = 1;
            cfg.CHAN = 1;
            cfg.OPTION.transmissionPower = TXPOWER_20;
            cfg.OPTION.ioDriveMode = IO_PUSH_PULL;
            cfg.OPTION.fixedTransmission = TXMODE_TRANS;
            cfg.SPED.airDataRate = AIR_DATA_RATE_2M;
            cfg.SPED.uartBaudRate = UART_BPS_115200;
            cfg.SPED.uartParity = UART_PARITY_8N1;
            ebyte.setConfiguration(cfg);
        }
        else {
            term_println(c.status.desc());  // Description of code
        }
    }
    else {
        term_printf("[EBYTE] Initialized fail!");
    }
}

// ----------------------------------------------------------------------------
void ebyte_process() {
    if (millis() > ebyte_period_millis) {
        term_println("\n[EBYTE] >>>");

        ebyte_period_millis = millis() + EBYTE_PERIOD;  // Reset timeout
    }
}
