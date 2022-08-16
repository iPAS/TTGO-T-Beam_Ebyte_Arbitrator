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
    // EBYTE_SERIAL.setRxBufferSize(EBYTE_UART_BUFFER_SIZE);
    // EBYTE_SERIAL.setTimeout(EBYTE_UART_BUFFER_TMO);
    // EBYTE_SERIAL.begin(EBYTE_BAUD, SERIAL_8N1, EBYTE_PIN_RX, EBYTE_PIN_TX);
    // while (!EBYTE_SERIAL)
    //     vTaskDelay(1);  // Yield
    // while (EBYTE_SERIAL.available())
    //     EBYTE_SERIAL.read();  // Clear buffer
    if (!ebyte.begin()) {  // Start communication with Ebyte module: config & etc.
        term_printf("[EBYTE] Init failed!");
    }
}

// ----------------------------------------------------------------------------
void ebyte_process() {
    if (millis() > ebyte_period_millis) {
        term_println("\n[EBYTE] >>>");
        ResponseStructContainer c;
        c = ebyte.getConfiguration();
        Configuration configuration = *((Configuration *)c.data);  // It's important get configuration pointer before all other operation

        term_println(c.status.desc());  // Description of code
        term_println(c.status.code);  // 1 if Success
        // printParameters(configuration);

        c.close();
        ebyte_period_millis = millis() + EBYTE_PERIOD;  // Reset timeout
    }
}
