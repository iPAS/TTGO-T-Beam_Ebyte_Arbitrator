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
#define EBYTE_PIN_E34_RX 13 // RX to Ebyte TX
#define EBYTE_PIN_E34_TX 2  // TX to Ebyte RX
#define EBYTE_PIN_AUX   34
#define EBYTE_PIN_M0    25
#define EBYTE_PIN_M1    14

Ebyte_E34 ebyte(&EBYTE_SERIAL, EBYTE_PIN_AUX, EBYTE_PIN_M0, EBYTE_PIN_M1, EBYTE_PIN_E34_RX, EBYTE_PIN_E34_TX);

#define EBYTE_REPORT_PERIOD_MS 10000
int ebyte_show_report_count = 0;  // 0 is 'disable', -1 is 'forever', other +n will be counted down to zero.
bool ebyte_loopback_flag = false;
uint8_t ebyte_airrate_level = 2;  // 0=250kbps | 1=1Mbps | 2=2Mbps
uint8_t ebyte_txpower_level = 0;  // 0=20dBm | 1=14dBm | 2=8dBm | 3=2dBm


// ----------------------------------------------------------------------------
void ebyte_setup() {
    // Setup as a modem connected to computer
    computer.setRxBufferSize(EBYTE_FC_RX_BUFFER_SIZE);
    computer.begin(EBYTE_FC_BAUD, SERIAL_8N1, EBYTE_FC_PIN_RX, EBYTE_FC_PIN_TX);
    computer.setTimeout(EBYTE_FC_UART_TMO);
    while (!computer) taskYIELD();  // Yield
    while (computer.available())
        computer.read();  // Clear buffer

    // Ebyte setup
    if (ebyte.begin()) {  // Start communication with Ebyte module: config & etc.
        term_println(F(ENDL "[EBYTE] Initialized successfully"));

        ResponseStructContainer rc;
        rc = ebyte.getConfiguration();  // Get c.data from here
        Configuration cfg = *((Configuration *)rc.data); // This is a memory transfer, NOT by-reference.
        rc.close();  // Clean c.data that was allocated in ::getConfiguration()

        if (rc.status.code == E34_SUCCESS){

            //
            // Old configuration
            //
            term_println(F("[EBYTE] Old configuration"));
            ebyte.printParameters(&cfg);

            //
            // Setup the desired mode
            //
            cfg.ADDH = EBYTE_BROADCAST_ADDR & 0x0F;  // No re-sending
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

            //
            // Recheck
            //
            rc = ebyte.getConfiguration();  // Get c.data from here
            cfg = *((Configuration *)rc.data); // This is a memory transfer, NOT by-reference.
            rc.close();

            if (rc.status.code == E34_SUCCESS){
                term_println(F("[EBYTE] New configuration"));
                ebyte.printParameters(&cfg);
            }
            else {
                term_print(F("[EBYTE] Re-checking failed!, E34: "));
                term_println(rc.status.desc());  // Description of code
            }

            // Change the baudrate to data transfer rate.
            ebyte.changeBpsRate(EBYTE_BAUD);
        }
        else {
            term_print(F("[EBYTE] Reading old configuration failed!, E34: "));
            term_println(rc.status.desc());  // Description of code
        }
    }
    else {
        term_println(F("[EBYTE] Initialized fail!"));
    }
}

// ----------------------------------------------------------------------------
void ebyte_process() {
    static uint32_t report_millis = millis() + EBYTE_REPORT_PERIOD_MS;
    static uint32_t downlink_byte_sum = 0;
    static uint32_t uplink_byte_sum = 0;
    static uint32_t prev_arival_millis = 0;         // Previous time the packet came
    static uint32_t inter_arival_sum_millis = 0;    // Cummulative sum of inter-packet arival time
    static uint32_t inter_arival_count = 0;

    //
    // Uplink -- Ebyte to Computer
    //
    if (ebyte.available()) {
        uint32_t arival_millis = millis();  // Arival timestamp
        inter_arival_sum_millis += arival_millis - prev_arival_millis;
        inter_arival_count++;
        prev_arival_millis = arival_millis;

        ResponseContainer rc = ebyte.receiveMessage();
        const char * p = rc.data.c_str();
        size_t len = rc.data.length();

        if (rc.status.code != E34_SUCCESS) {
            term_print("[EBYTE] E2C error, E34: ");
            term_println(rc.status.desc());
        }
        else {

            //
            // Forward uplink
            //
            if (computer.write(p, len) != len) {
                term_println("[EBYTE] E2C error. Cannot write all");
            }
            else {
                if (system_verbose_level >= VERBOSE_INFO) {
                    term_printf("[EBYTE] Recv fm E34: %3d bytes", len);
                    if (system_verbose_level >= VERBOSE_DEBUG) {
                        term_println(" >> " + hex_stream(p, len));
                    }
                    else {
                        term_println();
                    }
                }
                uplink_byte_sum += len;  // Kepp stat
            }

            //
            // Loopback
            //
            if (ebyte_loopback_flag) {
                ResponseStatus resp_sts = ebyte.fragmentMessageQueueTx(p, len);

                if (resp_sts.code != E34_SUCCESS) {
                    term_printf("[EBYTE] Loopback error on enqueueing %d bytes, E34:", len);
                    term_println(resp_sts.desc());
                }
                else {
                    if (system_verbose_level >= VERBOSE_INFO) {
                        term_printf("[EBYTE] Loopback enqueueing %3d bytes" ENDL, len);
                    }
                }
            }

        }
    }

    //
    // Loopback -- to another Ebyte
    //  if all frame has been received  &&  fragments to be sent are in the queue
    //
    if ((ebyte.available() == 0) && ebyte.lengthMessageQueueTx()) {
        size_t len = ebyte.processMessageQueueTx();
        if (len == 0) {
            term_println(F("[EBYTE] Loopback error on sending queue!"));
        }
        else {
            if (system_verbose_level >= VERBOSE_DEBUG) {
                term_printf("[EBYTE] Loopback sending queue %3d bytes" ENDL, len);
            }
            downlink_byte_sum += len;  // Kepp stat
        }
    }

    //
    // Downlink -- Computer to Ebyte
    //
    else  // XXX: <-- Wait until all loopback frames are sent.
    if (computer.available()) {
        ResponseStatus resp_sts;
        resp_sts.code = ebyte.auxReady(EBYTE_NO_AUX_WAIT);

        // Forward downlink
        if (resp_sts.code == E34_SUCCESS)
        {
            byte buf[EBYTE_E34_MAX_LEN];
            size_t len = (computer.available() < EBYTE_E34_MAX_LEN)? computer.available() : EBYTE_E34_MAX_LEN;
            computer.readBytes(buf, len);

            resp_sts = ebyte.sendMessage(buf, len);
            if (resp_sts.code != E34_SUCCESS) {
                term_print("[EBYTE] C2E error, E34:");
                term_println(resp_sts.desc());
            }
            else {
                if (system_verbose_level >= VERBOSE_INFO) {
                    term_printf("[EBYTE] Send to E34: %3d bytes" ENDL, len);
                }
                downlink_byte_sum += len;  // Keep stat
            }
        }
        else {
            term_printf("[EBYTE] C2E error on waiting AUX HIGH, E34:");
            term_println(resp_sts.desc());
        }
    }

    //
    // Statistic calculation
    //
    uint32_t now = millis();
    if (now > report_millis) {
        if (ebyte_show_report_count > 0 || ebyte_show_report_count < 0) {
            float period = (EBYTE_REPORT_PERIOD_MS + (now - report_millis)) / 1000;  // Int. division
            float up_rate = uplink_byte_sum / period;
            float down_rate = downlink_byte_sum / period;  // per second

            char inter_arival_str[10];
            if (inter_arival_count > 0) {
                snprintf(inter_arival_str, sizeof(inter_arival_str), "%dms", inter_arival_sum_millis / inter_arival_count);
            } else {
                snprintf(inter_arival_str, sizeof(inter_arival_str), "--ms");
            }
            inter_arival_sum_millis = 0;
            inter_arival_count = 0;

            term_printf("[Ebyte] Report up:%.2fB/s down:%.2fB/s period:%.2fs inter_arival:%s" ENDL,
                up_rate, down_rate, period, inter_arival_str);

            if (ebyte_show_report_count > 0)
                ebyte_show_report_count--;
        }

        uplink_byte_sum = 0;
        downlink_byte_sum = 0;
        report_millis = now + EBYTE_REPORT_PERIOD_MS;
    }
}

// ----------------------------------------------------------------------------
/**
 * @brief Get configuration information.
 *
 * @return ResponseStructContainer
 */
ResponseStructContainer ebyte_get_configure(Configuration * cfg) {
    ResponseStructContainer rc = ebyte.getConfiguration();  // Get c.data from here
    // Configuration cfg = *((Configuration *)rc.data);  // This is a memory transfer, NOT by-reference.
    memcpy(cfg, rc.data, sizeof(Configuration));
    rc.close();  // Clean c.data that was allocated in ::getConfiguration()
    return rc;
}

/**
 * @brief Setup configuration via 'setter' callback function.
 *
 * @param level
 * @param callback_fn
 */
ResponseStructContainer ebyte_set_config(EbyteSetter & setter) {
    Configuration cfg;
    ResponseStructContainer rc = ebyte_get_configure(&cfg);
    if (rc.status.code == E34_SUCCESS) {  // Setting
        setter(&cfg);
        ebyte.setConfiguration(cfg);
    }
    return rc;
}

/**
 * @brief
 *
 */
void ebyte_set_airrate(uint8_t level) {
    ebyte.changeBpsRate(EBYTE_CONFIG_BAUD);  // Change the baudrate for configuring.

    // Setup
    class AirrateSetter: public EbyteSetter {
      public:
        AirrateSetter(uint8_t level): EbyteSetter(level) {};
        void operator ()(Configuration * cfg) {
            cfg->SPED.airDataRate = this->level;
            ebyte.setConfiguration(*cfg);
        };
    } setter(level);

    ResponseStructContainer rc = ebyte_set_config(setter);

    // Recheck
    if (rc.status.code == E34_SUCCESS) {
        Configuration cfg;
        rc = ebyte_get_configure(&cfg);

        if (cfg.SPED.airDataRate == level) {
            term_println(F("[EBYTE] ebyte_set_airrate() succeeded!"));
        }
        else {
            term_println(F("[EBYTE] ebyte_set_airrate() failed!"));
        }
    }
    else {
        term_print(F("[EBYTE] ebyte_set_airrate() failed!, E34: "));
        term_println(rc.status.desc());  // Description of code
    }

    ebyte.changeBpsRate(EBYTE_BAUD);  // Change the baudrate for data transfer.
}

/**
 * @brief
 *
 */
void ebyte_set_txpower(uint8_t level) {

}
