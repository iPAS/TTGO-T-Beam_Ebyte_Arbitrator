/**
 * @file ebyte_e34_def.h
 */
#ifndef __EBYTE_E34_DEF_H__
#define __EBYTE_E34_DEF_H__


#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


typedef enum RESPONSE_STATUS {
    E34_SUCCESS = 1,
    ERR_E34_UNKNOWN, /* something shouldn't happened */
    ERR_E34_NOT_SUPPORT,
    ERR_E34_NOT_IMPLEMENT,
    ERR_E34_NOT_INITIAL,
    ERR_E34_INVALID_PARAM,
    ERR_E34_DATA_SIZE_NOT_MATCH,
    ERR_E34_BUF_TOO_SMALL,
    ERR_E34_TIMEOUT,
    ERR_E34_HARDWARE,
    ERR_E34_HEAD_NOT_RECOGNIZED,
    ERR_E34_NO_RESPONSE_FROM_DEVICE,
    ERR_E34_WRONG_UART_CONFIG,
    ERR_E34_PACKET_TOO_BIG
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
    TRANSPARENT_TRANSMISSION = 0b0,
    FIXED_TRANSMISSION = 0b1
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
    POWER_20    = 0b00,
    POWER_14    = 0b01,
    POWER_8     = 0b10,
    POWER_2     = 0b11
};


#pragma pack(push, 1)

struct Speed {
    uint8_t airDataRate : 3;    // bit 0-2
    String airrate_desc() { return F("Not yet implemented!"); }

    uint8_t uartBaudRate : 3;   // bit 3-5
    String baudrate_desc() { return F("Not yet implemented!"); }

    uint8_t uartParity : 2;     // bit 6-7
    String parity_desc() { return F("Not yet implemented!"); }
};

struct Option {
    byte   transmissionPower : 2;   // bit 0-1
    String txpower_desc() { return F("Not yet implemented!"); }

    byte   fec : 1;                 // bit 2
    String fec_desc() { return F("Not yet implemented!"); }

    byte   wirelessWakeupTime : 3;  // bit 3-5
    String wl_wake_desc() { return F("Not yet implemented!"); }

    byte   ioDriveMode : 1;         // bit 6
    String io_drv_desc() { return F("Not yet implemented!"); }

    byte   fixedTransmission : 1;   // bit 7
    String fixed_tx_desc() { return F("Not yet implemented!"); }
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
            case E34_SUCCESS: return F("Success"); break;
            case ERR_E34_UNKNOWN: return F("Unknown"); break;
            case ERR_E34_NOT_SUPPORT: return F("Not support!"); break;
            case ERR_E34_NOT_IMPLEMENT: return F("Not implement"); break;
            case ERR_E34_NOT_INITIAL: return F("Not initial!"); break;
            case ERR_E34_INVALID_PARAM: return F("Invalid param!"); break;
            case ERR_E34_DATA_SIZE_NOT_MATCH: return F("Data size not match!"); break;
            case ERR_E34_BUF_TOO_SMALL: return F("Buff too small!"); break;
            case ERR_E34_TIMEOUT: return F("Timeout!!"); break;
            case ERR_E34_HARDWARE: return F("Hardware error!"); break;
            case ERR_E34_HEAD_NOT_RECOGNIZED: return F("Save mode returned not recognized!"); break;
            case ERR_E34_NO_RESPONSE_FROM_DEVICE: return F("No response from device! (Check wiring)"); break;
            case ERR_E34_WRONG_UART_CONFIG: return F("Wrong UART configuration! (BPS must be 9600 for configuration)"); break;
            case ERR_E34_PACKET_TOO_BIG: return F("The device support only 58byte of data transmission!"); break;
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


#endif  // __EBYTE_E34_DEF_H__
