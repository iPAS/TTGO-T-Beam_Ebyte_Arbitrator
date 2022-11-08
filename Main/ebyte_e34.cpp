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
#include "ebyte_e34.h"


#define EB E34


/**
 * @brief Constructor
 */
EbyteE34::EbyteE34(HardwareSerial * serial, byte auxPin, byte m0Pin, byte m1Pin, byte rxPin, byte txPin,  E34::REVISION rev)
        : EbyteModule(serial, auxPin, 0, NULL, rxPin, txPin) {

    this->mPin_cnt = 2;
    this->mPins = new uint8_t[ this->mPin_cnt ];
    this->mPins[0] = m0Pin;
    this->mPins[1] = m1Pin;
    for (int i = 0; i < this->mPin_cnt; i++) {
        pinMode(this->mPins[i], OUTPUT);
        digitalWrite(this->mPins[i], HIGH);
    }

    this->revision = rev;
}


/**
 * @brief Destroy the Ebyte E 3 4:: Ebyte E 3 4 object
 */
EbyteE34::~EbyteE34() {
    delete [] this->mPins;
}

EbyteMode * EbyteE34::createMode(void) const {
    return new EbyteModeE34();
}

EbyteVersion * EbyteE34::createVersion(void) const {
    return new EbyteVersionE34();
}


/**
 * @brief Print debug
 */

void EbyteE34::printParameters(Configuration & config) const {
    this->printHead(config.getHead());

    term_print(F(" AddH   : ")); term_println(config.addr_msb, DEC);
    term_print(F(" AddL   : ")); term_println(config.addr_lsb, DEC);
    term_print(F(" Chan   : ")); term_println(config.channel,  DEC);

    EB::Speed * spd = (EB::Speed *)&config.speed;

    term_print(F(" Parity : ")); term_print(spd->uartParity,   BIN); term_print(" -> "); term_println(spd->parity_desc());
    term_print(F(" Baud   : ")); term_print(spd->uartBaudRate, BIN); term_print(" -> "); term_println(spd->baudrate_desc());
    term_print(F(" AirRate: ")); term_print(spd->airDataRate,  BIN); term_print(" -> "); term_println(spd->airrate_desc());

    EB::Option * opt = (EB::Option *)&config.option;

    term_print(F(" OpTxMod: ")); term_print(opt->fixedTransmission, BIN); term_print(" -> "); term_println(opt->fixed_tx_desc());
    term_print(F(" OpPlup : ")); term_print(opt->ioDriveMode,       BIN); term_print(" -> "); term_println(opt->io_drv_desc());
    term_print(F(" OpTxPow: ")); term_print(opt->transmissionPower, BIN); term_print(" -> "); term_println((this->revision == E34::D20)? opt->txpower_desc() : opt->txpower_desc_d27());

    term_println();
}


/**
 * @brief
 */

bool EbyteE34::addrChanToConfig(Configuration & config, bool changed, int32_t addr, int8_t chan) const {
    if (!changed) {  // No change, just comparing
        return (addr < 0  ||  ((config.addr_msb << 8) | config.addr_lsb) == addr
        )  &&  (chan < 0  ||  config.channel == chan
        );
    }

    if (addr >= 0) {
        config.addr_msb = (addr & 0x0000FFFF) >> 8;
        config.addr_lsb = (addr & 0x000000FF);
    }
    if (chan >= 0)
        config.channel = chan;
    return true;
}

bool EbyteE34::speedToConfig(Configuration & config, bool changed, int8_t air_baud, int8_t uart_baud, int8_t uart_parity) const {
    EB::Speed *spd = (EB::Speed *)&config.speed;

    if (!changed) {  // No change, just comparing
        return (air_baud    < 0  ||  spd->airDataRate == air_baud
        )  &&  (uart_baud   < 0  ||  spd->uartBaudRate == uart_baud
        )  &&  (uart_parity < 0  ||  spd->uartParity == uart_parity
        );
    }

    if (air_baud >= 0)
        spd->airDataRate = air_baud;
    if (uart_baud >= 0)
        spd->uartBaudRate = uart_baud;
    if (uart_parity >= 0)
        spd->uartParity = uart_parity;
    return true;
}

bool EbyteE34::optionToConfig(Configuration & config, bool changed, int8_t tx_pow, int8_t tx_mode, int8_t io_mode) const {
    EB::Option *opt = (EB::Option *)&config.option;

    if (!changed) {  // No change, just comparing
        return (tx_pow  < 0  ||  opt->transmissionPower == tx_pow
        )  &&  (tx_mode < 0  ||  opt->fixedTransmission == tx_mode
        )  &&  (io_mode < 0  ||  opt->ioDriveMode == io_mode
        );
    }

    if (tx_pow >= 0)
        opt->transmissionPower = tx_pow;
    if (tx_mode >= 0)
        opt->fixedTransmission = tx_mode;
    if (io_mode >= 0)
        opt->ioDriveMode = io_mode;
    return true;
}
