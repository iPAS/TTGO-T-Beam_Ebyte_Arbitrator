#include "global.h"


#define EBYTE_SERIAL    Serial2
#define EBYTE_BAUD      UART_BPS_RATE_115200
#define EBYTE_PIN_TX    2
#define EBYTE_PIN_RX    13
#define EBYTE_PIN_AUX   14
#define EBYTE_PIN_M0    36
#define EBYTE_PIN_M1    39
#define EBYTE_UART_BUFFER_SIZE 512
#define EBYTE_UART_BUFFER_TMO 1000

static LoRa_E32 ebyte(&EBYTE_SERIAL, EBYTE_PIN_AUX, EBYTE_PIN_M0, EBYTE_PIN_M1, EBYTE_BAUD);

#define EBYTE_PERIOD    5000
static uint32_t ebyte_period_millis = 0;


// ----------------------------------------------------------------------------
void ebyte_setup() {
    EBYTE_SERIAL.begin(EBYTE_BAUD, SERIAL_8N1, EBYTE_PIN_RX, EBYTE_PIN_TX);
    EBYTE_SERIAL.setRxBufferSize(EBYTE_UART_BUFFER_SIZE);
    EBYTE_SERIAL.setTimeout(EBYTE_UART_BUFFER_TMO);
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
        // It's important get configuration pointer before all other operation
        Configuration configuration = *((Configuration *)c.data);
        term_println(c.status.getResponseDescription()); // Description of code
        term_println(c.status.code); // 1 if Success

        ebyte_period_millis = millis() + EBYTE_PERIOD;  // Reset timeout
    }
}
