/**
 * @author Pasakorn Tiwatthanont (iPAS)
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Renzo Mischianti www.mischianti.org All right reserved.
 *
 * You may copy, alter and reuse this code in any way you like, but please leave
 * reference to www.mischianti.org in your comments if you redistribute this code.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef __EBYTE_MODULE_H__
#define __EBYTE_MODULE_H__


#include <Arduino.h>

#include "queue.h"
#include "helper.h"


// Uncomment to enable printing out nice debug messages.
#define EBYTE_DEBUG
#define EBYTE_LABEL "[EB] "

// Setup debug printing macros.
#ifdef EBYTE_DEBUG
#define DEBUG_PRINT(...) { term_print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...) { term_println(__VA_ARGS__); }
#define DEBUG_PRINTF(...) { term_printf(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#define DEBUG_PRINTF(...)
#endif


#define EBYTE_BROADCAST_ADDR 0xFF  // XXX: 0xFFFF or 0x0FFF please check

#define EBYTE_MODULE_BUFFER_SIZE 250  // 256 bytes at most

#define EBYTE_EXTRA_WAIT        40
#define EBYTE_NO_AUX_WAIT       100
#define EBYTE_RESPONSE_TMO      1000
#define EBYTE_CONFIG_BAUD       9600

#define EBYTE_UART_BUFFER_SIZE  512
#define EBYTE_UART_BUFFER_TMO   1000


typedef enum RESPONSE_STATUS {
    EB_SUCCESS = 1,
    EB_ERR_UNKNOWN, /* something shouldn't happened */
    EB_ERR_NOT_SUPPORT,
    EB_ERR_NOT_IMPLEMENT,
    EB_ERR_NOT_INITIAL,
    EB_ERR_INVALID_PARAM,
    EB_ERR_DATA_SIZE_NOT_MATCH,
    EB_ERR_BUF_TOO_SMALL,
    EB_ERR_TIMEOUT,
    EB_ERR_HARDWARE,
    EB_ERR_HEAD_NOT_RECOGNIZED,
    EB_ERR_NO_RESPONSE_FROM_DEVICE,
    EB_ERR_WRONG_UART_CONFIG,
    EB_ERR_PACKET_TOO_BIG
} Status;

enum MODE_TYPE {
    MODE_0_FIXED    = 0,
    MODE_1_HOPPING  = 1,
    MODE_2_RESERVED = 2,
    MODE_3_SLEEP    = 3,
    MODE_3_PROGRAM  = 3,
    MODE_INIT       = 0xFF
};

enum PROGRAM_COMMAND {
    WRITE_CFG_PWR_DWN_SAVE = 0xC0,
    READ_CONFIGURATION     = 0xC1,
    WRITE_CFG_PWR_DWN_LOSE = 0xC2,
    READ_MODULE_VERSION    = 0xC3,
    WRITE_RESET_MODULE     = 0xC4
};

enum UART_PARITY {
    UART_PARITY_8N1 = 0b00,
    UART_PARITY_8O1 = 0b01,
    UART_PARITY_8E1 = 0b10,
    UART_PARITY_8N1_ = 0b11
};

enum UART_BPS_RATE {
    UART_BPS_1200   = 0b000,
    UART_BPS_2400   = 0b001,
    UART_BPS_4800   = 0b010,
    UART_BPS_9600   = 0b011,
    UART_BPS_19200  = 0b100,
    UART_BPS_38400  = 0b101,
    UART_BPS_57600  = 0b110,
    UART_BPS_115200 = 0b111
};

enum AIR_DATA_RATE {
    AIR_DATA_RATE_250   = 0b000,
    AIR_DATA_RATE_1M    = 0b001,
    AIR_DATA_RATE_2M    = 0b010,
    AIR_DATA_RATE_2M_   = 0b011,
    // AIR_DATA_RATE_ = 0b100,
    // AIR_DATA_RATE_ = 0b101,
    // AIR_DATA_RATE_ = 0b110,
    // AIR_DATA_RATE_ = 0b111
};

enum TRANSMISSION_MODE {
    TXMODE_TRANS = 0b0,
    TXMODE_FIXED = 0b1
};

enum IO_DRIVE_MODE {
    IO_OPEN_COLLECTOR = 0b0,
    IO_PUSH_PULL = 0b1
};

// enum WIRELESS_WAKE_UP_TIME {
//     WAKE_UP_250  = 0b000,
//     WAKE_UP_500  = 0b001,
//     WAKE_UP_750  = 0b010,
//     WAKE_UP_1000 = 0b011,
//     WAKE_UP_1250 = 0b100,
//     WAKE_UP_1500 = 0b101,
//     WAKE_UP_1750 = 0b110,
//     WAKE_UP_2000 = 0b111
// };

// enum FORWARD_ERROR_CORRECTION_SWITCH {
//     FEC_0_OFF = 0b0,
//     FEC_1_ON = 0b1
// };

enum TRANSMISSION_POWER {
    TXPOWER_20    = 0b00,
    TXPOWER_14    = 0b01,
    TXPOWER_8     = 0b10,
    TXPOWER_2     = 0b11
};


#pragma pack(push, 1)

struct Speed {
    uint8_t airDataRate : 3;    // bit 0-2
    String airrate_desc() { 
        switch (this->airDataRate) {
            case 0:     return F("250kbps");
            case 1:     return F("1Mbps");
            case 2:     return F("2Mbps");
            case 3:     return F("2Mbps");
            default:    return F("N/A");
        }
    }

    uint8_t uartBaudRate : 3;   // bit 3-5
    String baudrate_desc() { 
        switch (this->uartBaudRate) {
            case 0:     return F("1200bps");
            case 1:     return F("2400bps");
            case 2:     return F("4800bps");
            case 3:     return F("9600bps");
            case 4:     return F("19200bps");
            case 5:     return F("38400bps");
            case 6:     return F("57600bps");
            case 7:     return F("115200bps");
            default:    return F("N/A");
        }
    }

    uint8_t uartParity : 2;     // bit 6-7
    String parity_desc() { 
        switch (this->uartParity) {
            case 0:     return F("8N1");
            case 1:     return F("8O1");
            case 2:     return F("8E1");
            case 3:     return F("8N1");
            default:    return F("N/A");
        }
    }
};

struct Option {
    byte   transmissionPower : 2;   // bit 0-1
    String txpower_desc() { 
        switch (this->transmissionPower) {
            case 0:     return F("20dBm");
            case 1:     return F("14dBm");
            case 2:     return F("8dBm");
            case 3:     return F("2dBm");
            default:    return F("N/A");
        }
    }

    byte   fec : 1;                 // bit 2 -- Reserevd in E34
    byte   wirelessWakeupTime : 3;  // bit 3-5 -- Reserved in E34

    byte   ioDriveMode : 1;         // bit 6
    String io_drv_desc() { 
        switch (this->ioDriveMode) {
            case 0:     return F("AUX Open-Collector");
            case 1:     return F("AUX Push-Pull");
            default:    return F("N/A");
        }
    }

    byte   fixedTransmission : 1;   // bit 7
    String fixed_tx_desc() { 
        switch (this->fixedTransmission) {
            case 0:     return F("Trans");
            case 1:     return F("Fixed");
            default:    return F("N/A");
        }
    }
};

struct Configuration {
    byte          HEAD = 0;
    byte          ADDH = 0;
    byte          ADDL = 0;
    struct Speed  SPED;
    byte          CHAN = 0;
    struct Option OPTION;
};

struct ModuleInformation {
    byte HEAD      = 0;
    byte frequency = 0;
    byte version   = 0;
    byte features  = 0;
};

struct ResponseStatus {
    Status code;
    String desc() {
        switch (this->code) {
            case EB_SUCCESS:                   return F("Success");
            case EB_ERR_UNKNOWN:               return F("Unknown");
            case EB_ERR_NOT_SUPPORT:           return F("Not support!");
            case EB_ERR_NOT_IMPLEMENT:         return F("Not implement");
            case EB_ERR_NOT_INITIAL:           return F("Not initial!");
            case EB_ERR_INVALID_PARAM:         return F("Invalid param!");
            case EB_ERR_DATA_SIZE_NOT_MATCH:   return F("Data size not match!");
            case EB_ERR_BUF_TOO_SMALL:         return F("Buff too small!");
            case EB_ERR_TIMEOUT:               return F("Timeout!!");
            case EB_ERR_HARDWARE:              return F("Hardware error!");
            case EB_ERR_HEAD_NOT_RECOGNIZED:   return F("Save mode returned not recognized!");
            case EB_ERR_NO_RESPONSE_FROM_DEVICE: return F("No response from device! (Check wiring)");
            case EB_ERR_WRONG_UART_CONFIG:     return F("Wrong UART configuration! (BPS must be " STR(EBYTE_CONFIG_BAUD) " for configuration)");
            case EB_ERR_PACKET_TOO_BIG:        return F("Support only " STR(EBYTE_MODULE_BUFFER_SIZE) " bytes of data transmission!");
            default: return F("Invalid status!");
        }
    }
};

struct ResponseStructContainer {
    void *         data;
    ResponseStatus status;
    void           close() { free(this->data); }
};

struct ResponseContainer {
    String         data;
    ResponseStatus status;
};

typedef struct {
    byte ADDH = 0;
    byte ADDL = 0;
    byte CHAN = 0;
    unsigned char message[];
} FixedStransmission;

#pragma pack(pop)


/**
 * @brief Class Ebyte Interfacing
 *
 */
class EbyteModule {

  public:
    EbyteModule(HardwareSerial * serial, byte auxPin, uint8_t mPin_cnt, uint8_t * mPins, byte rxPin = -1, byte txPin = -1);
    ~EbyteModule();

    bool begin();

    Status    setMode(MODE_TYPE mode);
    MODE_TYPE getMode();

    ResponseStructContainer getConfiguration();
    ResponseStatus          setConfiguration(Configuration configuration, PROGRAM_COMMAND saveType = WRITE_CFG_PWR_DWN_LOSE);

    ResponseStructContainer getModuleInformation();
    ResponseStatus          resetModule();

    ResponseStatus          sendMessage(const void * message, size_t size);
    ResponseStatus          sendMessage(const String message);

    ResponseStatus          sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const void * message, size_t size);
    ResponseStatus          sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const String message);

    ResponseStatus          sendBroadcastFixedMessage(byte CHAN, const void * message, size_t size);
    ResponseStatus          sendBroadcastFixedMessage(byte CHAN, const String message);

    ResponseContainer       receiveMessage();
    ResponseStructContainer receiveMessageFixedSize(size_t size);
    ResponseContainer       receiveMessageUntil(char delimiter = '\0');
    ResponseContainer       receiveMessageString(size_t size);

    Status  sendStruct(const void * structureManaged, size_t size_of_st);
    Status  receiveStruct(void * structureManaged, size_t size_of_st);

    int     available();
    Status  auxReady(unsigned long timeout);

    uint32_t getBpsRate();
    void    changeBpsRate(uint32_t new_bps);

    void    printHead(byte HEAD);
    void    printParameters(struct Configuration * cfg);

    size_t          lengthMessageQueueTx();
    ResponseStatus  fragmentMessageQueueTx(const void * message, size_t size);
    size_t          processMessageQueueTx();

  protected:
    HardwareSerial * hs;
    uint32_t bpsRate = EBYTE_CONFIG_BAUD;
    uint32_t serialConfig = SERIAL_8N1;

    int8_t auxPin  = -1;

    uint8_t mPin_cnt = 0;
    uint8_t *mPins = NULL;

    int8_t rxPin   = -1;
    int8_t txPin   = -1;

    MODE_TYPE mode = MODE_0_FIXED;

    queue_t queueTx;

    void   managedDelay(unsigned long timeout);
    Status waitCompleteResponse(unsigned long timeout = EBYTE_RESPONSE_TMO, unsigned long waitNoAux = EBYTE_NO_AUX_WAIT);

    void flush();
    void cleanUARTBuffer();

    void writeProgramCommand(PROGRAM_COMMAND cmd);

    RESPONSE_STATUS checkUARTConfiguration(MODE_TYPE mode);
};


#endif  // __EBYTE_MODULE_H__
