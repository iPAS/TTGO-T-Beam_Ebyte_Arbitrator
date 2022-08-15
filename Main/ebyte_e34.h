/*
 * AUthOR: Pasakorn Tiwatthanont (iPAS)
 *
 * The source is originated from EBYTE LoRa E32 Series
 *
 * AUTHOR:  Renzo Mischianti
 * VERSION: 1.5.6
 *
 * https://www.mischianti.org/category/my-libraries/lora-e32-devices/
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
#ifndef __EBYTE_E34_H__
#define __EBYTE_E34_H__

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Uncomment to enable printing out nice debug messages.
#define EBYTE_DEBUG

// Setup debug printing macros.
#ifdef EBYTE_DEBUG
#define DEBUG_PRINT(...) { Serial.print(__VA_ARGS__); }
#define DEBUG_PRINTLN(...) { Serial.println(__VA_ARGS__); }
#else
#define DEBUG_PRINT(...) \
    {}
#define DEBUG_PRINTLN(...) \
    {}
#endif


#ifdef FREQUENCY_433
#define OPERATING_FREQUENCY 410
#elif defined(FREQUENCY_170)
#define OPERATING_FREQUENCY 130
#elif defined(FREQUENCY_470)
#define OPERATING_FREQUENCY 370
#elif defined(FREQUENCY_868)
#define OPERATING_FREQUENCY 862
#elif defined(FREQUENCY_915)
#define OPERATING_FREQUENCY 900
#else
#define OPERATING_FREQUENCY 410
#endif

#define BROADCAST_ADDRESS 0xFF

typedef enum RESPONSE_STATUS {
#ifndef ARDUINO_ARCH_STM32
    SUCCESS = 1,
#endif
    E32_SUCCESS = 1,
    ERR_E32_UNKNOWN, /* something shouldn't happened */
    ERR_E32_NOT_SUPPORT,
    ERR_E32_NOT_IMPLEMENT,
    ERR_E32_NOT_INITIAL,
    ERR_E32_INVALID_PARAM,
    ERR_E32_DATA_SIZE_NOT_MATCH,
    ERR_E32_BUF_TOO_SMALL,
    ERR_E32_TIMEOUT,
    ERR_E32_HARDWARE,
    ERR_E32_HEAD_NOT_RECOGNIZED,
    ERR_E32_NO_RESPONSE_FROM_DEVICE,
    ERR_E32_WRONG_UART_CONFIG,
    ERR_E32_PACKET_TOO_BIG
} Status;

static String getResponseDescriptionByParams(byte status) {
    switch (status) {
    case E32_SUCCESS: return F("Success"); break;
    case ERR_E32_UNKNOWN: return F("Unknown"); break;
    case ERR_E32_NOT_SUPPORT: return F("Not support!"); break;
    case ERR_E32_NOT_IMPLEMENT: return F("Not implement"); break;
    case ERR_E32_NOT_INITIAL: return F("Not initial!"); break;
    case ERR_E32_INVALID_PARAM: return F("Invalid param!"); break;
    case ERR_E32_DATA_SIZE_NOT_MATCH: return F("Data size not match!"); break;
    case ERR_E32_BUF_TOO_SMALL: return F("Buff too small!"); break;
    case ERR_E32_TIMEOUT: return F("Timeout!!"); break;
    case ERR_E32_HARDWARE: return F("Hardware error!"); break;
    case ERR_E32_HEAD_NOT_RECOGNIZED: return F("Save mode returned not recognized!"); break;
    case ERR_E32_NO_RESPONSE_FROM_DEVICE: return F("No response from device! (Check wiring)"); break;
    case ERR_E32_WRONG_UART_CONFIG: return F("Wrong UART configuration! (BPS must be 9600 for configuration)"); break;
    case ERR_E32_PACKET_TOO_BIG: return F("The device support only 58byte of data transmission!"); break;
    default: return F("Invalid status!");
    }
}

enum UART_PARITY { MODE_00_8N1 = 0b00, MODE_01_8O1 = 0b01, MODE_10_8E1 = 0b10, MODE_11_8N1 = 0b11 };

static String getUARTParityDescriptionByParams(byte uartParity) {
    switch (uartParity) {
    case MODE_00_8N1: return F("8N1 (Default)"); break;
    case MODE_01_8O1: return F("8O1"); break;
    case MODE_10_8E1: return F("8E1"); break;
    case MODE_11_8N1: return F("8N1"); break;
    default: return F("Invalid UART Parity!");
    }
}

enum UART_BPS_TYPE {
    UART_BPS_1200   = 0b000,
    UART_BPS_2400   = 0b001,
    UART_BPS_4800   = 0b010,
    UART_BPS_9600   = 0b011,
    UART_BPS_19200  = 0b100,
    UART_BPS_38400  = 0b101,
    UART_BPS_57600  = 0b110,
    UART_BPS_115200 = 0b111
};

enum UART_BPS_RATE {
    UART_BPS_RATE_1200   = 1200,
    UART_BPS_RATE_2400   = 2400,
    UART_BPS_RATE_4800   = 4800,
    UART_BPS_RATE_9600   = 9600,
    UART_BPS_RATE_19200  = 19200,
    UART_BPS_RATE_38400  = 38400,
    UART_BPS_RATE_57600  = 57600,
    UART_BPS_RATE_115200 = 115200
};

static String getUARTBaudRateDescriptionByParams(byte uartBaudRate) {
    switch (uartBaudRate) {
    case UART_BPS_1200: return F("1200bps"); break;
    case UART_BPS_2400: return F("2400bps"); break;
    case UART_BPS_4800: return F("4800bps"); break;
    case UART_BPS_9600: return F("9600bps (default)"); break;
    case UART_BPS_19200: return F("19200bps"); break;
    case UART_BPS_38400: return F("38400bps"); break;
    case UART_BPS_57600: return F("57600bps"); break;
    case UART_BPS_115200: return F("115200bps"); break;
    default: return F("Invalid UART Baud Rate!");
    }
}

enum AIR_DATA_RATE {
    AIR_DATA_RATE_000_03  = 0b000,
    AIR_DATA_RATE_001_12  = 0b001,
    AIR_DATA_RATE_010_24  = 0b010,
    AIR_DATA_RATE_011_48  = 0b011,
    AIR_DATA_RATE_100_96  = 0b100,
    AIR_DATA_RATE_101_192 = 0b101,
    AIR_DATA_RATE_110_192 = 0b110,
    AIR_DATA_RATE_111_192 = 0b111
};

static String getAirDataRateDescriptionByParams(byte airDataRate) {
    switch (airDataRate) {
    case AIR_DATA_RATE_000_03: return F("0.3kbps"); break;
    case AIR_DATA_RATE_001_12: return F("1.2kbps"); break;
    case AIR_DATA_RATE_010_24: return F("2.4kbps (default)"); break;
    case AIR_DATA_RATE_011_48: return F("4.8kbps"); break;
    case AIR_DATA_RATE_100_96: return F("9.6kbps"); break;
    case AIR_DATA_RATE_101_192: return F("19.2kbps"); break;
    case AIR_DATA_RATE_110_192: return F("19.2kbps"); break;
    case AIR_DATA_RATE_111_192: return F("19.2kbps"); break;
    default: return F("Invalid Air Data Rate!");
    }
}

enum FIDEX_TRANSMISSION { FT_TRANSPARENT_TRANSMISSION = 0b0, FT_FIXED_TRANSMISSION = 0b1 };

static String getFixedTransmissionDescriptionByParams(byte fixedTransmission) {
    switch (fixedTransmission) {
    case FT_TRANSPARENT_TRANSMISSION: return F("Transparent transmission (default)"); break;
    case FT_FIXED_TRANSMISSION:
        return F("Fixed transmission (first three bytes can be used as high/low address and channel)");
        break;
    default: return F("Invalid fixed transmission param!");
    }
}

enum IO_DRIVE_MODE { IO_D_MODE_OPEN_COLLECTOR = 0b0, IO_D_MODE_PUSH_PULLS_PULL_UPS = 0b1 };

static String getIODriveModeDescriptionDescriptionByParams(byte ioDriveMode) {
    switch (ioDriveMode) {
    case IO_D_MODE_OPEN_COLLECTOR: return F("TXD, RXD, AUX are open-collectors"); break;
    case IO_D_MODE_PUSH_PULLS_PULL_UPS: return F("TXD, RXD, AUX are push-pulls/pull-ups"); break;
    default: return F("Invalid IO drive mode!");
    }
}

enum WIRELESS_WAKE_UP_TIME {
    WAKE_UP_250  = 0b000,
    WAKE_UP_500  = 0b001,
    WAKE_UP_750  = 0b010,
    WAKE_UP_1000 = 0b011,
    WAKE_UP_1250 = 0b100,
    WAKE_UP_1500 = 0b101,
    WAKE_UP_1750 = 0b110,
    WAKE_UP_2000 = 0b111
};

static String getWirelessWakeUPTimeDescriptionByParams(byte wirelessWakeUPTime) {
    switch (wirelessWakeUPTime) {
    case WAKE_UP_250: return F("250ms (default)"); break;
    case WAKE_UP_500: return F("500ms"); break;
    case WAKE_UP_750: return F("750ms"); break;
    case WAKE_UP_1000: return F("1000ms"); break;
    case WAKE_UP_1250: return F("1250ms"); break;
    case WAKE_UP_1500: return F("1500ms"); break;
    case WAKE_UP_1750: return F("1750ms"); break;
    case WAKE_UP_2000: return F("2000ms"); break;
    default: return F("Invalid wireless wake-up mode!");
    }
}
enum FORWARD_ERROR_CORRECTION_SWITCH { FEC_0_OFF = 0b0, FEC_1_ON = 0b1 };

static String getFECDescriptionByParams(byte fec) {
    switch (fec) {
    case FEC_0_OFF: return F("Turn off Forward Error Correction Switch"); break;
    case FEC_1_ON: return F("Turn on Forward Error Correction Switch (Default)"); break;
    default: return F("Invalid FEC param");
    }
}

enum TRANSMISSION_POWER {
    POWER_20 = 0b00,
    POWER_17 = 0b01,
    POWER_14 = 0b10,
    POWER_10 = 0b11

};

static String getTransmissionPowerDescriptionByParams(byte transmissionPower) {
    switch (transmissionPower) {
    case POWER_20: return F("20dBm (Default)"); break;
    case POWER_17: return F("17dBm"); break;
    case POWER_14: return F("14dBm"); break;
    case POWER_10: return F("10dBm"); break;
    default: return F("Invalid transmission power param");
    }
}


#define MAX_SIZE_TX_PACKET 29

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


#pragma pack(push, 1)
struct Speed {
    uint8_t airDataRate : 3;  // bit 0-2
    String  getAirDataRate() { return getAirDataRateDescriptionByParams(this->airDataRate); }

    uint8_t uartBaudRate : 3;  // bit 3-5
    String  getUARTBaudRate() { return getUARTBaudRateDescriptionByParams(this->uartBaudRate); }

    uint8_t uartParity : 2;  // bit 6-7
    String  getUARTParityDescription() { return getUARTParityDescriptionByParams(this->uartParity); }
};

struct Option {
    byte   transmissionPower : 2;  // bit 0-1
    String getTransmissionPowerDescription() {
        return getTransmissionPowerDescriptionByParams(this->transmissionPower);
    }

    byte   fec : 1;  // bit 2
    String getFECDescription() { return getFECDescriptionByParams(this->fec); }

    byte   wirelessWakeupTime : 3;  // bit 3-5
    String getWirelessWakeUPTimeDescription() {
        return getWirelessWakeUPTimeDescriptionByParams(this->wirelessWakeupTime);
    }

    byte   ioDriveMode : 1;  // bit 6
    String getIODroveModeDescription() { return getIODriveModeDescriptionDescriptionByParams(this->ioDriveMode); }

    byte   fixedTransmission : 1;  // bit 7
    String getFixedTransmissionDescription() {
        return getFixedTransmissionDescriptionByParams(this->fixedTransmission);
    }
};

struct Configuration {
    byte          HEAD = 0;
    byte          ADDH = 0;
    byte          ADDL = 0;
    struct Speed  SPED;
    byte          CHAN = 0;
    String        getChannelDescription() { return String(this->CHAN + OPERATING_FREQUENCY) + F("MHz"); }
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
    String getResponseDescription() { return getResponseDescriptionByParams(this->code); }
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
#pragma pack(pop)


/**
 * @brief Class Ebyte E34 Interfacing
 *
 */
class Ebyte_E34 {
  public:
    Ebyte_E34(HardwareSerial * serial,                                      UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);
    Ebyte_E34(HardwareSerial * serial, byte auxPin,                         UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);
    Ebyte_E34(HardwareSerial * serial, byte auxPin, byte m0Pin, byte m1Pin, UART_BPS_RATE bpsRate = UART_BPS_RATE_9600);

    Ebyte_E34(byte txE32pin, byte rxE32pin, HardwareSerial * serial,                                      UART_BPS_RATE bpsRate, uint32_t serialConfig = SERIAL_8N1);
    Ebyte_E34(byte txE32pin, byte rxE32pin, HardwareSerial * serial, byte auxPin,                         UART_BPS_RATE bpsRate, uint32_t serialConfig = SERIAL_8N1);
    Ebyte_E34(byte txE32pin, byte rxE32pin, HardwareSerial * serial, byte auxPin, byte m0Pin, byte m1Pin, UART_BPS_RATE bpsRate, uint32_t serialConfig = SERIAL_8N1);

    bool begin();

    Status    setMode(MODE_TYPE mode);
    MODE_TYPE getMode();

    ResponseStructContainer getConfiguration();
    ResponseStatus          setConfiguration(Configuration configuration, PROGRAM_COMMAND saveType = WRITE_CFG_PWR_DWN_LOSE);

    ResponseStructContainer getModuleInformation();
    ResponseStatus          resetModule();

    ResponseStatus          sendMessage(const void * message, const uint8_t size);
    ResponseStatus          sendMessage(const String message);

    ResponseStatus          sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const String message);
    ResponseStatus          sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const void * message, const uint8_t size);
    ResponseStatus          sendBroadcastFixedMessage(byte CHAN, const void * message, const uint8_t size);
    ResponseStatus          sendBroadcastFixedMessage(byte CHAN, const String message);

    ResponseContainer       receiveMessageUntil(char delimiter = '\0');
    ResponseStructContainer receiveMessage(const uint8_t size);
    ResponseContainer       receiveMessage();

    ResponseContainer       receiveInitialMessage(const uint8_t size);

    int available();

  private:
    HardwareSerial * hs;
    uint32_t serialConfig = SERIAL_8N1;
    UART_BPS_RATE bpsRate = UART_BPS_RATE_9600;

    int8_t txE32pin = -1;
    int8_t rxE32pin = -1;
    int8_t auxPin   = -1;
    int8_t m0Pin    = -1;
    int8_t m1Pin    = -1;

    struct NeedsStream {
        template <typename T> void begin(T & t, uint32_t baud) {
            DEBUG_PRINTLN("Begin ");
            t.setTimeout(500);
            t.begin(baud);
            stream = &t;
        }

        template <typename T> void begin(T & t, uint32_t baud, uint32_t config) {
            DEBUG_PRINTLN("Begin ");
            t.setTimeout(500);
            t.begin(baud, config);
            stream = &t;
        }

        template <typename T> void begin(T & t, uint32_t baud, uint32_t config, int8_t txE32pin, int8_t rxE32pin) {
            DEBUG_PRINTLN("Begin ");
            t.setTimeout(500);
            t.begin(baud, config, txE32pin, rxE32pin);
            stream = &t;
        }

        void listen() {}

        Stream * stream;
    };
    NeedsStream serialDef;

    MODE_TYPE mode = MODE_0_FIXED;

    void   managedDelay(unsigned long timeout);
    Status waitCompleteResponse(unsigned long timeout = 1000, unsigned int waitNoAux = 100);

    void flush();
    void cleanUARTBuffer();

    Status sendStruct(void * structureManaged, uint16_t size_);
    Status receiveStruct(void * structureManaged, uint16_t size_);

    void writeProgramCommand(PROGRAM_COMMAND cmd);

    RESPONSE_STATUS checkUARTConfiguration(MODE_TYPE mode);

    #ifdef EBYTE_DEBUG
    void printParameters(struct Configuration * configuration);
    #endif
};

#endif  // __EBYTE_E32_H__
