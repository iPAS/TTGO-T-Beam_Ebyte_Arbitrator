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

#include "ebyte_e34_def.h"


/**
 * @brief Class Ebyte E34 Interfacing
 *
 */
class Ebyte_E34 {
  public:
    Ebyte_E34(HardwareSerial * serial, byte auxPin, byte m0Pin, byte m1Pin, byte rxPin = -1, byte txPin = -1);

    bool begin();

    Status    setMode(MODE_TYPE mode);
    MODE_TYPE getMode();

    ResponseStructContainer getConfiguration();
    ResponseStatus          setConfiguration(Configuration configuration, PROGRAM_COMMAND saveType = WRITE_CFG_PWR_DWN_LOSE);

    ResponseStructContainer getModuleInformation();
    ResponseStatus          resetModule();

    ResponseStatus          sendMessage(const void * message, const uint8_t size);
    ResponseStatus          sendMessage(const String message);

    ResponseStatus          sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const void * message, const uint8_t size);
    ResponseStatus          sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const String message);

    ResponseStatus          sendBroadcastFixedMessage(byte CHAN, const void * message, const uint8_t size);
    ResponseStatus          sendBroadcastFixedMessage(byte CHAN, const String message);

    ResponseContainer       receiveMessage();
    ResponseStructContainer receiveMessageFixedSize(uint8_t size);
    ResponseContainer       receiveMessageUntil(char delimiter = '\0');
    ResponseContainer       receiveMessageString(uint8_t size);

    Status sendStruct(const void * structureManaged, uint16_t size_);
    Status receiveStruct(void * structureManaged, uint16_t size_);

    int available();

    uint32_t getBpsRate();
    void changeBpsRate(uint32_t new_bps);

    void printHead(byte HEAD);
    void printParameters(struct Configuration * cfg);

  private:
    HardwareSerial * hs;
    uint32_t bpsRate = EBYTE_CONFIG_BAUD;
    uint32_t serialConfig = SERIAL_8N1;

    int8_t auxPin  = -1;
    int8_t m0Pin   = -1;
    int8_t m1Pin   = -1;
    int8_t rxPin   = -1;
    int8_t txPin   = -1;

    MODE_TYPE mode = MODE_0_FIXED;

    void   managedDelay(unsigned long timeout);
    Status waitCompleteResponse(unsigned long timeout = 1000, unsigned int waitNoAux = 100);

    void flush();
    void cleanUARTBuffer();

    void writeProgramCommand(PROGRAM_COMMAND cmd);

    RESPONSE_STATUS checkUARTConfiguration(MODE_TYPE mode);
};


#endif  // __EBYTE_E34_H__
