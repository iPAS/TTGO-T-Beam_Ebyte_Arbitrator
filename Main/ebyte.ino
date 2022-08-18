#include "global.h"


// Computer config
#define computer        EBYTE_FC_SERIAL
#define EBYTE_FC_SERIAL Serial1
#define EBYTE_FC_BAUD   115200
#define EBYTE_FC_PIN_RX 4   // 15
#define EBYTE_FC_PIN_TX 23  // 12
#define EBYTE_FC_RX_BUFFER_SIZE 512
#define EBYTE_FC_UART_TMO 1000

// Ebyte config
#define EBYTE_SERIAL    Serial2
#define EBYTE_BAUD      115200
#define EBYTE_PIN_E34_RX 13 // TX to Ebyte RX
#define EBYTE_PIN_E34_TX 2  // RX to Ebyte TX
#define EBYTE_PIN_AUX   34
#define EBYTE_PIN_M0    25
#define EBYTE_PIN_M1    14
#define EBYTE_UART_BUFFER_SIZE 512
#define EBYTE_UART_BUFFER_TMO 1000

Ebyte_E34 ebyte(&EBYTE_SERIAL, EBYTE_PIN_AUX, EBYTE_PIN_M0, EBYTE_PIN_M1, EBYTE_PIN_E34_RX, EBYTE_PIN_E34_TX);

#define EBYTE_REPORT_PERIOD_MS 10000
static uint32_t report_millis;
static uint32_t downlink_byte_sum = 0;
static uint32_t uplink_byte_sum = 0;
int ebyte_show_report_count = 0;  // 0 is 'disable', -1 is 'forever', other +n will be counted down to zero.

bool ebyte_loopback_flag = false;


// ----------------------------------------------------------------------------
void ebyte_setup() {
    // Setup as a modem connected to computer
    computer.begin(EBYTE_FC_BAUD, SERIAL_8N1, EBYTE_FC_PIN_RX, EBYTE_FC_PIN_TX);
    computer.setRxBufferSize(EBYTE_FC_RX_BUFFER_SIZE);
    computer.setTimeout(EBYTE_FC_UART_TMO);
    while (!computer) taskYIELD();  // Yield
    while (computer.available())
        computer.read();  // Clear buffer

    // Ebyte setup
    if (ebyte.begin()) {  // Start communication with Ebyte module: config & etc.
        term_println(ENDL "[EBYTE] Initialized successfully");

        ResponseStructContainer resp;
        resp = ebyte.getConfiguration();  // Get c.data from here
        Configuration cfg = *((Configuration *)resp.data); // This is a memory transfer, NOT by-reference.
        resp.close();  // Clean c.data that was allocated in ::getConfiguration()

        if (resp.status.code == E34_SUCCESS){
            term_println(F("[EBYTE] Old configuration"));
            ebyte.printParameters(&cfg);

            // Setup the desired mode
            cfg.ADDH = EBYTE_BROADCAST_ADDR;// & 0x0F;  // No re-sending
            cfg.ADDL = EBYTE_BROADCAST_ADDR;
            cfg.CHAN = 6;  // XXX: 2.508 GHz -- out of WiFi channels
            cfg.OPTION.transmissionPower = TXPOWER_20;
            cfg.OPTION.ioDriveMode      = IO_PUSH_PULL;
            cfg.OPTION.fixedTransmission = TXMODE_TRANS;  // XXX:
            cfg.SPED.airDataRate        = AIR_DATA_RATE_2M;
            cfg.SPED.uartBaudRate       = UART_BPS_115200;  // XXX: don't forget to ::changeBpsRate( EBYTE_BAUD )
            cfg.SPED.uartParity         = UART_PARITY_8N1;
            ebyte.setConfiguration(cfg);
            // ebyte.setConfiguration(cfg, WRITE_CFG_PWR_DWN_SAVE);  // XXX: Save

            // Recheck
            resp = ebyte.getConfiguration();  // Get c.data from here
            cfg = *((Configuration *)resp.data); // This is a memory transfer, NOT by-reference.
            resp.close();

            if (resp.status.code == E34_SUCCESS){
                term_println(F("[EBYTE] New configuration"));
                ebyte.printParameters(&cfg);
            }
            else {
                term_println(resp.status.desc());  // Description of code
            }

            // Change the baudrate for data transfer.
            ebyte.changeBpsRate(EBYTE_BAUD);
        }
        else {
            term_println(resp.status.desc());  // Description of code
        }
    }
    else {
        term_printf("[EBYTE] Initialized fail!" ENDL);
    }

    // Periods setup
    report_millis = millis() + EBYTE_REPORT_PERIOD_MS;
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
            term_printf("[EBYTE] send to E34: %d bytes" ENDL, len);
            downlink_byte_sum += len;  // Keep stat
        }
    }

    if (ebyte.available()) {
        ResponseContainer rc = ebyte.receiveMessage();
        const char * p = rc.data.c_str();
        uint16_t len = rc.data.length();
        if (rc.status.code != E34_SUCCESS) {
            term_print("[EBYTE] E2C error, E34: ");
            term_println(rc.status.desc());
        }
        else
        if (ebyte_loopback_flag) {

            uplink_byte_sum += len;  // Kepp stat

            ResponseStatus status = ebyte.sendMessage(p, len);
            if (status.code != E34_SUCCESS) {
                term_print("[EBYTE] E2E error, E34:");
                term_println(status.desc());
            }
            else {
                term_printf("[EBYTE] sendback to E34: %d bytes" ENDL, len);
                downlink_byte_sum += len;  // Kepp stat
            }

        }
        else
        if (computer.write(p, len) != len) {
            term_println("[EBYTE] E2C error. Cannot write all");
        }
        else {
            term_printf("[EBYTE] recv from E34: %d bytes" ENDL, len);
            term_println(" >> " + hex_stream(p, len));
            uplink_byte_sum += len;  // Kepp stat
        }
    }

    uint32_t now = millis();
    if (now > report_millis) {
        float period = EBYTE_REPORT_PERIOD_MS + (now - report_millis);
        float up_rate = (uplink_byte_sum * 1000) / period;
        float down_rate = (downlink_byte_sum * 1000) / period;  // per second

        if (ebyte_show_report_count > 0 || ebyte_show_report_count < 0) {
            // term_printf("[CLI] Ebyte report up:%d down:%d" ENDL, uplink_byte_sum, downlink_byte_sum);
            term_printf("[CLI] Ebyte report up:%.2fB/s down:%.2fB/s period:%.2fms" ENDL, up_rate, down_rate, period);

            if (ebyte_show_report_count > 0)
                ebyte_show_report_count--;
        }

        uplink_byte_sum = 0;
        downlink_byte_sum = 0;
        report_millis = now + EBYTE_REPORT_PERIOD_MS;
    }
}
