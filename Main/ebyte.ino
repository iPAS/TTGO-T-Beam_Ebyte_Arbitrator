#include "global.h"


// Computer config
#define computer        EBYTE_FC_SERIAL
#define EBYTE_FC_SERIAL Serial1

#if EBYTE_MODULE == EBYTE_E28
#define EBYTE_FC_BAUD   115200
#define EBYTE_NODE_ADDR 0xFFFF
#else
#define EBYTE_FC_BAUD   115200
#define EBYTE_NODE_ADDR 0x0FFF
#endif

#define EBYTE_FC_PIN_RX 4   // 21: RX to Flight-controller TX
#define EBYTE_FC_PIN_TX 23  // 22: TX to Flight-controller RX

#define EBYTE_FC_RX_BUFFER_SIZE EBYTE_UART_BUFFER_SIZE
// #define EBYTE_FC_UART_TMO       EBYTE_UART_BUFFER_TMO
#define EBYTE_FC_UART_TMO       40  // Quick enough to cut separately Mavlink frames.


// Ebyte config
#define EBYTE_SERIAL    Serial2

#if EBYTE_MODULE == EBYTE_E28
#define EBYTE_BAUD      115200
#define EBYTE_PIN_M2    32  // FIXME: Just pull-up 'M2' for now.
#else
#define EBYTE_BAUD      115200
#endif

#define EBYTE_PIN_RX    13  // RX to Ebyte TX
#define EBYTE_PIN_TX    2   // TX to Ebyte RX
#define EBYTE_PIN_AUX   34
#define EBYTE_PIN_M0    25
#define EBYTE_PIN_M1    14


#if EBYTE_MODULE == EBYTE_E34
EbyteE34 ebyte(&EBYTE_SERIAL, EBYTE_PIN_AUX, EBYTE_PIN_M0, EBYTE_PIN_M1, EBYTE_PIN_RX, EBYTE_PIN_TX);

#elif EBYTE_MODULE == EBYTE_E34D27
EbyteE34 ebyte(&EBYTE_SERIAL, EBYTE_PIN_AUX, EBYTE_PIN_M0, EBYTE_PIN_M1, EBYTE_PIN_RX, EBYTE_PIN_TX, E34::D27);

#elif EBYTE_MODULE == EBYTE_E28
EbyteE28 ebyte(&EBYTE_SERIAL, EBYTE_PIN_AUX, EBYTE_PIN_M0, EBYTE_PIN_M1, EBYTE_PIN_M2, EBYTE_PIN_RX, EBYTE_PIN_TX);

#endif


#define EBYTE_REPORT_PERIOD_MS 10000
int ebyte_show_report_count = 0;  // 0 is 'disable', -1 is 'forever', other +n will be counted down to zero.
#define EBYTE_LOOPBACK_TMO_MS 250
bool ebyte_loopback_flag = false;

uint8_t ebyte_airrate_level = 0;
uint8_t ebyte_txpower_level = 0;  // Maximum
uint8_t ebyte_channel = 6;
uint8_t ebyte_message_type = MSG_TYPE_RAW;


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
        term_printf(ENDL "[EBYTE] Initialized successfully for %s" ENDL, STR(EB));

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
            #if EBYTE_MODULE == EBYTE_E28
            ebyte.setAddrChanIntoConfig( cfg, EBYTE_NODE_ADDR, ebyte_channel);
            ebyte.setSpeedIntoConfig(    cfg, ebyte_airrate_level, EB::UART_BPS_115200, EB::UART_PARITY_8N1);
            #else
            ebyte.setAddrChanIntoConfig( cfg, EBYTE_NODE_ADDR, ebyte_channel);
            ebyte.setSpeedIntoConfig(    cfg, ebyte_airrate_level, EB::UART_BPS_115200, EB::UART_PARITY_8N1);
            #endif
            ebyte.setOptionIntoConfig(   cfg, ebyte_txpower_level, EB::TXMODE_TRANS, EB::IO_PUSH_PULL);
            ebyte.setConfiguration(cfg);  // XXX: pass 'WRITE_CFG_PWR_DWN_SAVE' to save on Ebyte's EEPROM

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
                term_print(F("[EBYTE] Re-checking failed!, "));
                term_println(rc.status.desc());  // Description of code
            }

            // Change the baudrate to data transfer rate.
            ebyte.setBpsRate(EBYTE_BAUD);
        }
        else {
            term_print(F("[EBYTE] Reading old configuration failed!, "));
            term_println(rc.status.desc());  // Description of code
        }
    }
    else {
        term_println(F("[EBYTE] Initialized fail!"));
    }
}

// ----------------------------------------------------------------------------
byte * ebyte_mavlink_segmentor(byte * p, size_t len, size_t *new_len) {

    *new_len = len;
    return p;
}

// ----------------------------------------------------------------------------
void ebyte_uplink_process(ebyte_stat_t *s) {
    if (ebyte.available()) {
        uint32_t arival_millis = millis();  // Arival timestamp
        s->inter_arival_sum_millis += arival_millis - s->prev_arival_millis;
        s->inter_arival_count++;
        s->prev_arival_millis = arival_millis;

        ResponseContainer rc = ebyte.receiveMessage();
        byte * p = (byte *)rc.data.c_str();
        size_t len = rc.data.length();

        if (rc.status.code != ResponseStatus::SUCCESS) {
            term_print("[EBYTE] E2C error!, ");
            term_println(rc.status.desc());
        }
        else {

            ////////////////////////////////////////////
            // Preprocess depends on the message mode //
            ////////////////////////////////////////////
            if (ebyte_message_type == MSG_TYPE_RAW) {
                // Passthrough
            }
            else
            if (ebyte_message_type == MSG_TYPE_MAVLINK) {
                p = ebyte_mavlink_segmentor(p, len, &len);
            }

            ////////////////////
            // Forward uplink //
            ////////////////////
            if (computer.write(p, len) != len) {
                term_println("[EBYTE] E2C error. Cannot write all");
            }
            else {
                if (system_verbose_level >= VERBOSE_INFO) {
                    term_printf("[EBYTE] Recv: %3d bytes", len);
                    if (system_verbose_level >= VERBOSE_DEBUG) {
                        term_println(" >> " + hex_stream(p, len));
                    }
                    else {
                        term_println();
                    }
                }
                s->uplink_byte_sum += len;  // Kepp stat
            }

            ///////////////////////////
            // Loopback, on this end //
            ///////////////////////////
            if (ebyte_loopback_flag) {
                s->loopback_tmo_millis = arival_millis + EBYTE_LOOPBACK_TMO_MS;
                ResponseStatus status = ebyte.fragmentMessageQueueTx(p, len);  // In-queuing to be sent sequentially

                if (status.code != ResponseStatus::SUCCESS) {
                    term_printf("[EBYTE] Loopback error on enqueueing %d bytes, ", len);
                    term_println(status.desc());
                }
                else {
                    if (system_verbose_level >= VERBOSE_INFO) {
                        term_printf("[EBYTE] Loopback enqueueing %3d bytes, q size %d" ENDL, len, ebyte.lengthMessageQueueTx());
                    }
                }
            }

        }
    }
}

// ----------------------------------------------------------------------------
void ebyte_downlink_process(ebyte_stat_t *s) {
    //////////////////////////////////
    // Loopback, to the another end //
    //////////////////////////////////
    // if no more data to be queued, and queue is ready.
    if (ebyte.available() == 0
    &&  ebyte.lengthMessageQueueTx() > 0
    &&  millis() > s->loopback_tmo_millis) {
        size_t len = ebyte.processMessageQueueTx();  // Send out the loopback frames

        if (len == 0) {
            term_println(F("[EBYTE] Loopback error on sending queue!"));
        }
        else {
            if (system_verbose_level >= VERBOSE_DEBUG) {
                term_printf("[EBYTE] Loopback sending queue %3d bytes, q size %d" ENDL, len, ebyte.lengthMessageQueueTx());
            }
            s->downlink_byte_sum += len;  // Kepp stat
        }
    }

    else  // We prefer the loopback frames to be consecutive; so, using 'else' here.

    //////////////////////
    // Forward downlink //
    //////////////////////
    // from upper to lower, if no more loopback queued frame.
    if (computer.available()) {
        ResponseStatus status;
        status = ebyte.auxReady(EBYTE_NO_AUX_WAIT);

        // Forward downlink
        if (status.code == ResponseStatus::SUCCESS)
        {
            byte buf[EBYTE_MODULE_BUFFER_SIZE];
            size_t len = (computer.available() < ARRAY_SIZE(buf))? computer.available() : ARRAY_SIZE(buf);
            computer.readBytes(buf, len);

            status = ebyte.sendMessage(buf, len);
            if (status.code != ResponseStatus::SUCCESS) {
                term_print("[EBYTE] C2E error, ");
                term_println(status.desc());
            }
            else {
                if (system_verbose_level >= VERBOSE_INFO) {
                    term_printf("[EBYTE] Send: %3d bytes" ENDL, len);
                }
                s->downlink_byte_sum += len;  // Keep stat
            }
        }
        else {
            term_printf("[EBYTE] C2E error on waiting AUX HIGH, ");
            term_println(status.desc());
        }
    }
}

// ----------------------------------------------------------------------------
void ebyte_process() {
    static ebyte_stat_t stat {};

    //
    // Uplink -- Ebyte to Computer
    //
    ebyte_uplink_process(&stat);

    //
    // Downlink -- Computer to Ebyte
    //
    ebyte_downlink_process(&stat);

    //
    // Statistic calculation
    //
    uint32_t now = millis();
    if (now > stat.report_millis) {
        if (ebyte_show_report_count > 0 || ebyte_show_report_count < 0) {
            float period = (EBYTE_REPORT_PERIOD_MS + (now - stat.report_millis)) / 1000;  // Int. division
            float up_rate = stat.uplink_byte_sum / period;
            float down_rate = stat.downlink_byte_sum / period;  // per second

            char inter_arival_str[10];
            if (stat.inter_arival_count > 0) {
                snprintf(inter_arival_str, sizeof(inter_arival_str), "%dms", stat.inter_arival_sum_millis / stat.inter_arival_count);
            } else {
                snprintf(inter_arival_str, sizeof(inter_arival_str), "--ms");
            }
            stat.inter_arival_sum_millis = 0;
            stat.inter_arival_count = 0;

            term_printf("[Ebyte] Report up:%.2fB/s down:%.2fB/s period:%.2fs inter_arival:%s" ENDL,
                up_rate, down_rate, period, inter_arival_str);

            if (ebyte_show_report_count > 0)
                ebyte_show_report_count--;
        }

        stat.uplink_byte_sum = 0;
        stat.downlink_byte_sum = 0;
        stat.report_millis = now + EBYTE_REPORT_PERIOD_MS;
    }
}

// ----------------------------------------------------------------------------
/**
 * @brief Get configuration information.
 */
ResponseStructContainer ebyte_get_config(Configuration & config) {
    ResponseStructContainer rc = ebyte.getConfiguration();  // Get c.data from here
    // Configuration config = *((Configuration *)rc.data);  // This is a memory transfer, NOT by-reference.
    memcpy(&config, rc.data, sizeof(Configuration));
    rc.close();  // Clean c.data that was allocated in ::getConfiguration()
    return rc;
}

/**
 * @brief Setup configuration via 'setter' callback function.
 */
ResponseStructContainer ebyte_set_config(EbyteSetter & setter) {
    Configuration cfg;
    ResponseStructContainer rc = ebyte_get_config(cfg);
    if (rc.status.code == ResponseStatus::SUCCESS) {  // Setting
        setter(cfg);
        ebyte.setConfiguration(cfg);
    }
    return rc;
}

/**
 * @brief ebyte_setter
 */
void ebyte_set_configs(EbyteSetter & setter) {
    ebyte.setBpsRate(EBYTE_CONFIG_BAUD);  // Change the baudrate for configuring.

    // Setting
    ResponseStructContainer rc = ebyte_set_config(setter);

    // Validate
    if (rc.status.code == ResponseStatus::SUCCESS) {
        Configuration cfg;
        rc = ebyte_get_config(cfg);

        if (setter.validate(cfg) == true) {
            term_println(F("[EBYTE] setter.validate() succeeded!"));
        }
        else {
            term_println(F("[EBYTE] setter.validate() failed!"));
        }
    }
    else {
        term_print(F("[EBYTE] ebyte_set_config() failed!, "));
        term_println(rc.status.desc());  // Description of code
    }

    ebyte.setBpsRate(EBYTE_BAUD);  // Change the baudrate for data transfer.
}

/**
 * @brief
 */
void ebyte_apply_configs() {
    class Setter: public EbyteSetter {
      public:
        Setter(uint8_t param): EbyteSetter(param) {};

        void operator () (Configuration & config) {
            ebyte.setAddrChanIntoConfig(config, -1, ebyte_channel);
            ebyte.setSpeedIntoConfig(   config, ebyte_airrate_level, -1, -1);
            ebyte.setOptionIntoConfig(  config, ebyte_txpower_level, -1, -1);
            ebyte.setConfiguration(config);
        };

        bool validate(Configuration & config) {
            return (ebyte.compareAddrChan(config, -1, ebyte_channel)
                &&  ebyte.compareSpeed(   config, ebyte_airrate_level, -1, -1)
                &&  ebyte.compareOption(  config, ebyte_txpower_level, -1, -1)
            );
        };
    } setter(0);

    ebyte_set_configs(setter);
}

/**
 * @brief
 */
void ebyte_set_channel(uint8_t chan) {
    class Setter: public EbyteSetter {
      public:
        Setter(uint8_t param): EbyteSetter(param) {};

        void operator () (Configuration & config) {
            ebyte.setAddrChanIntoConfig(config, -1, this->byte_param);
            ebyte.setConfiguration(config);
        };

        bool validate(Configuration & config) {
            return ebyte.compareAddrChan(config, -1, this->byte_param);
        };
    } setter(chan);

    ebyte_set_configs(setter);
}

/**
 * @brief
 */
void ebyte_set_airrate(uint8_t level) {
    class Setter: public EbyteSetter {
      public:
        Setter(uint8_t param): EbyteSetter(param) {};

        void operator () (Configuration & config) {
            ebyte.setSpeedIntoConfig(config, this->byte_param, -1, -1);
            ebyte.setConfiguration(config);
        };

        bool validate(Configuration & config) {
            return ebyte.compareSpeed(config, this->byte_param, -1, -1);
        };
    } setter(level);

    ebyte_set_configs(setter);
}

/**
 * @brief
 */
void ebyte_set_txpower(uint8_t level) {
    class Setter: public EbyteSetter {
      public:
        Setter(uint8_t param): EbyteSetter(param) {};

        void operator () (Configuration & config) {
            ebyte.setOptionIntoConfig(config, this->byte_param, -1, -1);
            ebyte.setConfiguration(config);
        };

        bool validate(Configuration & config) {
            return ebyte.compareOption(config, this->byte_param, -1, -1);
        };
    } setter(level);

    ebyte_set_configs(setter);
}
