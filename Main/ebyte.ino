#include "global.h"

#include <EByte_LoRa_E32_library.h>


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

#define EBYTE_PERIOD    1000
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
    // if (SERIAL_V.available()) {
    //     line = SERIAL_V.readStringUntil('\n');  // The terminator character is discarded from the serial buffer.
    //                                             // '\n' NOT included.

    //     // Filter-out unused information
    //     for (i = 0; i < line.length();) {       // Cut some response
    //         j = line.indexOf('\r', i);          // Separated with '\r'
    //         if (j < 0) break;
    //         if (j > i) {                        // Ignore a single '\r'.
    //             sub = line.substring(i, j);     // @ [j] NOT included
    //             // term_printf("  %d,%d\t%s", i, j-1, sub.c_str());  // XXX: for debugging

    //             // Filtering mechanism
    //             if (sub[0] == '$') 
    //                 ;  // Skip echo
    //             else
    //                 buffer += sub + '\n';
    //         }
    //         i = j+1;  // Next char left
    //     }

    // }

    if (millis() > ebyte_period_millis) {
        term_println(c.getResponseDescription()); // Description of code
        term_println(c.code); // 1 if Success
        
        ebyte_period_millis = millis() + EBYTE_PERIOD;  // Reset timeout
    }
}
