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
#ifndef __EBYTE_E34_H__
#define __EBYTE_E34_H__


#include <Arduino.h>
#include "ebyte_module.h"


/**
 * @brief Class Ebyte configuration
 *
 */

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
};

enum TRANSMISSION_MODE {
    TXMODE_TRANS = 0b0,
    TXMODE_FIXED = 0b1
};

enum IO_DRIVE_MODE {
    IO_OPEN_COLLECTOR = 0b0,
    IO_PUSH_PULL = 0b1
};

enum TRANSMISSION_POWER {
    TXPOWER_20    = 0b00,
    TXPOWER_14    = 0b01,
    TXPOWER_8     = 0b10,
    TXPOWER_2     = 0b11
};


/**
 * @brief EbyteVersionE34
 *
 */
class EbyteVersionE34 : public EbyteVersion {

  public:
    EbyteVersionE34() : EbyteVersion(4) {}
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
 * @brief EbyteModeE34
 *
 */
class EbyteModeE34 : public EbyteMode {
    void setModeDefault()   override { this->code = 0; }
    void setModeConfig()    override { this->code = 3; }
    bool isModeConfig()     override { return this->code == 3; }
    bool isModeCorrect()    override { return this->code <= 3; }
    String description()    override {
        switch (this->code) {
            case 0: return F("Fixed frequency mode");
            case 1: return F("Frequency hopping mode");
            case 2: return F("Reservation mode");
            case 3: return F("Sleep/Setting mode");
        }
        return F("Invalid mode!");
    }
};


/**
 * @brief EbyteE34
 *
 */
class EbyteE34 : public EbyteModule {

  public:
    EbyteE34(HardwareSerial * serial, byte auxPin, byte m0Pin, byte m1Pin, byte rxPin = -1, byte txPin = -1);
    ~EbyteE34();

  protected:
    EbyteMode * createMode(void) const override;
    EbyteVersion * createVersion(void) const override;
};


#endif  // __EBYTE_E34_H__
