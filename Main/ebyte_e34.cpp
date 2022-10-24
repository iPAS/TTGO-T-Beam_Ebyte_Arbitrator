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


/**
 * @brief Constructor
 */
EbyteE34::EbyteE34(HardwareSerial * serial, byte auxPin, byte m0Pin, byte m1Pin, byte rxPin, byte txPin)
        : EbyteModule(serial, auxPin, 0, NULL, rxPin, txPin) {

    this->mPin_cnt = mPin_cnt;
    this->mPins = new uint8_t[mPin_cnt];
    for (int i = 0; i < mPin_cnt; i++) {
        this->mPins[i] = mPins[i];
        pinMode(this->mPins[i], OUTPUT);
        digitalWrite(this->mPins[i], HIGH);
    }
}


/**
 * @brief Destroy the Ebyte E 3 4:: Ebyte E 3 4 object
 */
EbyteE34::~EbyteE34() {
    delete [] this->mPins;
}
