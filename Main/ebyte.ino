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

EbyteE34 ebyte(&EBYTE_SERIAL, EBYTE_PIN_AUX, EBYTE_PIN_M0, EBYTE_PIN_M1, EBYTE_PIN_E34_RX, EBYTE_PIN_E34_TX);

#define EBYTE_REPORT_PERIOD_MS 10000
int ebyte_show_report_count = 0;  // 0 is 'disable', -1 is 'forever', other +n will be counted down to zero.
bool ebyte_loopback_flag = false;
uint8_t ebyte_airrate_level = 2;  // 0=250kbps | 1=1Mbps | 2=2Mbps
uint8_t ebyte_txpower_level = 0;  // 0=20dBm | 1=14dBm | 2=8dBm | 3=2dBm
uint8_t ebyte_channel = 6;  // 0-11 where ch6 = 2.508 GHz -- out of WiFi channels


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

        if (rc.status.code == ResponseStatus::SUCCESS){

            //
            // Old configuration
            //
            term_println(F("[EBYTE] Old configuration"));
            ebyte.printParameters(cfg);

            //
            // Setup the desired mode
            //
            cfg.addr_msb = EBYTE_BROADCAST_ADDR & 0x0F;  // No re-sending
            cfg.addr_lsb = EBYTE_BROADCAST_ADDR;
            cfg.channel = ebyte_channel;  // ch6 = 2.508 GHz -- out of WiFi channels
            cfg.OPTION.transmissionPower    = ebyte_txpower_level;  // TXPOWER_20;
            cfg.OPTION.ioDriveMode          = IO_PUSH_PULL;
            cfg.OPTION.fixedTransmission    = TXMODE_TRANS;         // no special bytes leading
            cfg.SPED.airDataRate            = ebyte_airrate_level;  // AIR_DATA_RATE_2M;
            cfg.SPED.uartBaudRate           = UART_BPS_115200;      // XXX: don't forget to ::setBpsRate( EBYTE_BAUD )
            cfg.SPED.uartParity             = UART_PARITY_8N1;
            ebyte.setConfiguration(cfg);
            // ebyte.setConfiguration(cfg, WRITE_CFG_PWR_DWN_SAVE);  // XXX: Save on Ebyte's EEPROM

            //
            // Recheck
            //
            rc = ebyte.getConfiguration();  // Get c.data from here
            cfg = *((Configuration *)rc.data); // This is a memory transfer, NOT by-reference.
            rc.close();

            if (rc.status.code == ResponseStatus::SUCCESS){
                term_println(F("[EBYTE] New configuration"));
                ebyte.printParameters(cfg);
            }
            else {
                term_print(F("[EBYTE] Re-checking failed!, E34: "));
                term_println(rc.status.desc());  // Description of code
            }

            // Change the baudrate to data transfer rate.
            ebyte.setBpsRate(EBYTE_BAUD);
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

        if (rc.status.code != ResponseStatus::SUCCESS) {
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
                ResponseStatus status = ebyte.fragmentMessageQueueTx(p, len);

                if (status.code != ResponseStatus::SUCCESS) {
                    term_printf("[EBYTE] Loopback error on enqueueing %d bytes, E34:", len);
                    term_println(status.desc());
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
        ResponseStatus status;
        status = ebyte.auxReady(EBYTE_NO_AUX_WAIT);

        // Forward downlink
        if (status.code == ResponseStatus::SUCCESS)
        {
            byte buf[EBYTE_MODULE_BUFFER_SIZE];
            size_t len = (computer.available() < EBYTE_MODULE_BUFFER_SIZE)? computer.available() : EBYTE_MODULE_BUFFER_SIZE;
            computer.readBytes(buf, len);

            status = ebyte.sendMessage(buf, len);
            if (status.code != ResponseStatus::SUCCESS) {
                term_print("[EBYTE] C2E error, E34:");
                term_println(status.desc());
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
            term_println(status.desc());
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
ResponseStructContainer ebyte_get_config(Configuration * cfg) {
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
    ResponseStructContainer rc = ebyte_get_config(&cfg);
    if (rc.status.code == ResponseStatus::SUCCESS) {  // Setting
        setter(&cfg);
        ebyte.setConfiguration(cfg);
    }
    return rc;
}

/**
 * @brief ebyte_setter
 *
 */
void ebyte_set_configs(EbyteSetter & setter) {
    ebyte.setBpsRate(EBYTE_CONFIG_BAUD);  // Change the baudrate for configuring.

    // Setting
    ResponseStructContainer rc = ebyte_set_config(setter);

    // Validate
    if (rc.status.code == ResponseStatus::SUCCESS) {
        Configuration cfg;
        rc = ebyte_get_config(&cfg);

        if (setter.validate(&cfg) == true) {
            term_println(F("[EBYTE] setter.validate() succeeded!"));
        }
        else {
            term_println(F("[EBYTE] setter.validate() failed!"));
        }
    }
    else {
        term_print(F("[EBYTE] ebyte_set_config() failed!, E34: "));
        term_println(rc.status.desc());  // Description of code
    }

    ebyte.setBpsRate(EBYTE_BAUD);  // Change the baudrate for data transfer.
}

/**
 * @brief
 *
 */
void ebyte_apply_configs() {
    class Setter: public EbyteSetter {
      public:
        Setter(uint8_t param): EbyteSetter(param) {};

        void operator ()(Configuration * cfg) {
            cfg->SPED.airDataRate = ebyte_airrate_level;
            cfg->OPTION.transmissionPower = ebyte_txpower_level;
            cfg->channel = ebyte_channel;
            ebyte.setConfiguration(*cfg);
        };

        bool validate(Configuration * cfg) {
            return (cfg->SPED.airDataRate         == ebyte_airrate_level  &&
                    cfg->OPTION.transmissionPower == ebyte_txpower_level  &&
                    cfg->channel                     == ebyte_channel
                    )? true : false;
        };
    } setter(0);

    ebyte_set_configs(setter);
}

/**
 * @brief
 *
 */
void ebyte_set_airrate(uint8_t level) {
    class Setter: public EbyteSetter {
      public:
        Setter(uint8_t param): EbyteSetter(param) {};

        void operator ()(Configuration * cfg) {
            cfg->SPED.airDataRate = this->byte_param;
            ebyte.setConfiguration(*cfg);
        };

        bool validate(Configuration * cfg) {
            return (cfg->SPED.airDataRate == this->byte_param)? true : false;
        };
    } setter(level);

    ebyte_set_configs(setter);
}

/**
 * @brief
 *
 */
void ebyte_set_txpower(uint8_t level) {
    class Setter: public EbyteSetter {
      public:
        Setter(uint8_t param): EbyteSetter(param) {};

        void operator ()(Configuration * cfg) {
            cfg->OPTION.transmissionPower = this->byte_param;
            ebyte.setConfiguration(*cfg);
        };

        bool validate(Configuration * cfg) {
            return (cfg->OPTION.transmissionPower == this->byte_param)? true : false;
        };
    } setter(level);

    ebyte_set_configs(setter);
}

/**
 * @brief
 *
 */
void ebyte_set_channel(uint8_t ch) {
    class Setter: public EbyteSetter {
      public:
        Setter(uint8_t param): EbyteSetter(param) {};

        void operator ()(Configuration * cfg) {
            cfg->channel = this->byte_param;
            ebyte.setConfiguration(*cfg);
        };

        bool validate(Configuration * cfg) {
            return (cfg->channel == this->byte_param)? true : false;
        };
    } setter(ch);

    ebyte_set_configs(setter);
}
