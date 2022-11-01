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
#ifndef __EBYTE_E28_H__
#define __EBYTE_E28_H__


#include <Arduino.h>
#include "ebyte_module.h"


/**
 * @brief Class Ebyte configuration
 *
 */

enum UART_PARITY {
    UART_PARITY_8N1  = 0b00,
    UART_PARITY_8O1  = 0b01,
    UART_PARITY_8E1  = 0b10,
    UART_PARITY_8N1_ = 0b11
};

enum UART_BPS_RATE {
    UART_BPS_1200   = 0b000,
    UART_BPS_4800   = 0b001,
    UART_BPS_9600   = 0b010,
    UART_BPS_19200  = 0b011,
    UART_BPS_57600  = 0b100,
    UART_BPS_115200 = 0b101,
    UART_BPS_460800 = 0b110,
    UART_BPS_921600 = 0b111
};

enum AIR_DATA_RATE {
    AIR_RATE_AUTO = 0b000,
    AIR_RATE_1K   = 0b001,
    AIR_RATE_5K   = 0b010,
    AIR_RATE_10K  = 0b011,
    AIR_RATE_50K  = 0b100,
    AIR_RATE_100K = 0b101,
    AIR_RATE_1M   = 0b110,
    AIR_RATE_2M   = 0b111,
};

enum TRANSMISSION_MODE {
    TXMODE_TRANS = 0b0,
    TXMODE_FIXED = 0b1
};

enum IO_DRIVE_MODE {
    IO_OPEN_COLLECTOR = 0b0,
    IO_PUSH_PULL      = 0b1
};

enum TRANSMISSION_POWER {
    TXPOWER_12 = 0b00,
    TXPOWER_10 = 0b01,
    TXPOWER_7  = 0b10,
    TXPOWER_4  = 0b11
};


#pragma pack(push, 1)

struct Speed {
    uint8_t airDataRate : 3;    // bit 0-2
    String airrate_desc() {
        switch (this->airDataRate) {
            case 0: return F("auto");
            case 1: return F("1kbps");
            case 2: return F("5kbps");
            case 3: return F("10kbps");
            case 4: return F("50kbps");
            case 5: return F("100kbps");
            case 6: return F("1Mbps");
            case 7: return F("2Mbps");
            default: return F("N/A");
        }
    }

    uint8_t uartBaudRate : 3;   // bit 3-5
    String baudrate_desc() {
        switch (this->uartBaudRate) {
            case 0: return F("1200bps");
            case 1: return F("4800bps");
            case 2: return F("9600bps");
            case 3: return F("19200bps");
            case 4: return F("57600bps");
            case 5: return F("115200bps");
            case 6: return F("460800bps");
            case 7: return F("921600bps");
            default: return F("N/A");
        }
    }

    uint8_t uartParity : 2;     // bit 6-7
    String parity_desc() {
        switch (this->uartParity) {
            case 0: return F("8N1");
            case 1: return F("8O1");
            case 2: return F("8E1");
            case 3: return F("8N1");
            default: return F("N/A");
        }
    }
};


struct Option {
    byte   transmissionPower : 2;   // bit 0-1
    String txpower_desc() {
        switch (this->transmissionPower) {
            case 0: return F("12dBm");
            case 1: return F("10dBm");
            case 2: return F("7dBm");
            case 3: return F("4dBm");
            default: return F("N/A");
        }
    }

    byte   ioDriveMode : 1;         // bit 2
    String io_drv_desc() {
        switch (this->ioDriveMode) {
            case 0: return F("AUX Open-Collector");
            case 1: return F("AUX Push-Pull");
            default: return F("N/A");
        }
    }

    byte   switchLBT : 1;           // bit 3
    byte   _reserved : 3;           // bit 4-6

    byte   fixedTransmission : 1;   // bit 7
    String fixed_tx_desc() {
        switch (this->fixedTransmission) {
            case 0: return F("Trans");
            case 1: return F("Fixed");
            default: return F("N/A");
        }
    }
};

#pragma pack(pop)


/**
 * @brief EbyteVersionE28
 *
 */
class EbyteVersionE28 : public EbyteVersion {

  public:
    EbyteVersionE28() : EbyteVersion(4) {}
    String getInfo(void) {
        struct Version {
            byte HEAD;
            byte series_no;
            byte version_no;
            byte features;
        } * p = (Version *)this->data;

        char str[30];
        snprintf(str, sizeof(str), "series(%02X) ver(%02X) feat(%02X)", p->series_no, p->version_no, p->features);
        return String(str);
    }
};


/**
 * @brief EbyteModeE28
 *
 */
class EbyteModeE28 : public EbyteMode {
    void setModeDefault()   override { this->code = 0+4; }
    void setModeConfig()    override { this->code = 3+4; }
    bool isModeConfig()     override { return this->code == 3+4; }
    bool isModeCorrect()    override { return this->code <= 3+4; }
    String description()    override {
        switch (this->code) {
            case 0+4: return F("Transmission mode");
            case 1+4: return F("RSSI mode");
            case 2+4: return F("Reservation mode");
            case 3+4: return F("Sleep/Setting mode");
            case 0: return F("Low-power mode");
        }
        return F("Invalid mode!");
    }
};


/**
 * @brief EbyteE28
 *
 */
class EbyteE28 : public EbyteModule {

  public:
    EbyteE28(HardwareSerial * serial, byte auxPin, byte m0Pin, byte m1Pin, byte m2Pin, byte rxPin = -1, byte txPin = -1);
    ~EbyteE28();

    bool addrChanToConfig(  Configuration & config,
                            bool changed,           // Whether comparing only or setting
                            int32_t addr = -1,      // 4-bit MSB -- retry count
                            int8_t chan = -1        // ch6 = 2.508 GHz -- out of WiFi channels
                            ) const override;
    bool speedToConfig(     Configuration & config,
                            bool changed,           // Whether comparing only or setting
                            int8_t air_baud = -1,   // AIR_DATA_RATE_2M
                            int8_t uart_baud = -1,  // UART_BPS_115200
                            int8_t uart_parity = -1 // UART_PARITY_8N1
                            ) const override;
    bool optionToConfig(    Configuration & config,
                            bool changed,           // Whether comparing only or setting
                            int8_t tx_pow = -1,     // TXPOWER_20
                            int8_t tx_mode = -1,    // TXMODE_TRANS
                            int8_t io_mode = -1     // IO_PUSH_PULL
                            ) const override;

    void printParameters(Configuration & config) const override;

  protected:
    EbyteMode * createMode(void) const override;
    EbyteVersion * createVersion(void) const override;
};


#endif  // __EBYTE_E28_H__
