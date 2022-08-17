#include "global.h"


// As a flow-controller, config
#define computer        EBYTE_FC_SERIAL
#define EBYTE_FC_SERIAL Serial1
#define EBYTE_FC_BAUD   115200
#define EBYTE_FC_PIN_RX 15
#define EBYTE_FC_PIN_TX 12
#define EBYTE_FC_RX_BUFFER_SIZE 512
#define EBYTE_FC_UART_TMO 1000

// Ebyte config
#define EBYTE_SERIAL    Serial2
#define EBYTE_BAUD      115200
#define EBYTE_PIN_E34_RX 2   // uC TX
#define EBYTE_PIN_E34_TX 13  // uC RX
#define EBYTE_PIN_AUX   34
#define EBYTE_PIN_M0    25
#define EBYTE_PIN_M1    14
#define EBYTE_UART_BUFFER_SIZE 512
#define EBYTE_UART_BUFFER_TMO 1000

Ebyte_E34 ebyte(&EBYTE_SERIAL, EBYTE_PIN_AUX, EBYTE_PIN_M0, EBYTE_PIN_M1, EBYTE_PIN_E34_RX, EBYTE_PIN_E34_TX);

// ----------------------------------------------------------------------------
void ebyte_setup() {
    // Setup as a modem
    EBYTE_FC_SERIAL.begin(EBYTE_FC_BAUD, SERIAL_8N1, EBYTE_FC_PIN_RX, EBYTE_FC_PIN_TX);
    EBYTE_FC_SERIAL.setRxBufferSize(EBYTE_FC_RX_BUFFER_SIZE);
    EBYTE_FC_SERIAL.setTimeout(EBYTE_FC_UART_TMO);
    while (!EBYTE_FC_SERIAL) taskYIELD();  // Yield
    while (EBYTE_FC_SERIAL.available())
        EBYTE_FC_SERIAL.read();  // Clear buffer

    // Ebyte setup
    if (ebyte.begin()) {  // Start communication with Ebyte module: config & etc.
        term_println("\n[EBYTE] Initialized successfully");

        ResponseStructContainer c;
        c = ebyte.getConfiguration();  // Get c.data from here
        Configuration cfg = *((Configuration *)c.data); // This is a memory transfer, NOT by-reference.
                                                        // It's important get configuration pointer before all other operation.
        c.close();  // Clean c.data that was allocated in ::getConfiguration()

        if (c.status.code == E34_SUCCESS){
            ebyte.printParameters(&cfg);

            // Setup the desired mode
            cfg.ADDH = EBYTE_BROADCAST_ADDR;// & 0x0F;  // No re-sending
            cfg.ADDL = EBYTE_BROADCAST_ADDR;
            cfg.CHAN = 6;  // XXX: 2.508 GHz -- out of WiFi channels
            cfg.OPTION.transmissionPower = TXPOWER_20;
            cfg.OPTION.ioDriveMode      = IO_PUSH_PULL;
            cfg.OPTION.fixedTransmission = TXMODE_TRANS;  // XXX:
            cfg.SPED.airDataRate        = AIR_DATA_RATE_2M;
            cfg.SPED.uartBaudRate       = UART_BPS_115200;
            cfg.SPED.uartParity         = UART_PARITY_8N1;
            ebyte.setConfiguration(cfg);
            // ebyte.setConfiguration(cfg, WRITE_CFG_PWR_DWN_SAVE);  // XXX: Save

            
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
    if (computer.available()) {
        byte buf[EBYTE_E34_MAX_LEN];
        uint8_t len = (computer.available() < EBYTE_E34_MAX_LEN)? computer.available() : EBYTE_E34_MAX_LEN;
        computer.readBytes(buf, len);

        ResponseStatus status = ebyte.sendMessage(buf, len);
        if (status.code != E34_SUCCESS) {
            term_print("[EBYTE] C2E error, E34:");
            term_println(status.desc());
        }
        else {
            term_printf("[EBYTE] send to E34: %d bytes\n", len);
        }
    }

    if (ebyte.available()) {
        ResponseContainer rc = ebyte.receiveMessage();
        if (rc.status.code != E34_SUCCESS) {
            term_print("[EBYTE] E2C error, E34: ");
            term_println(rc.status.desc());
        }
        else
        if (Serial.write(rc.data.c_str(), rc.data.length()) != rc.data.length()) {
            term_println("[EBYTE] E2C error. Cannot write all");
        }
        else {
            term_printf("[EBYTE] recv from E34: %d bytes\n", rc.data.length());
        }
    }
}
