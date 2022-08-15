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
#include "ebyte_e34.h"


/**
 * @brief Construct a new Ebyte_E34::Ebyte_E34 object
 *
 */
Ebyte_E34::Ebyte_E34(HardwareSerial * serial, UART_BPS_RATE bpsRate) {
    this->hs = serial;
    this->bpsRate = bpsRate;
}

Ebyte_E34::Ebyte_E34(HardwareSerial * serial, byte auxPin, UART_BPS_RATE bpsRate) {
    this->hs = serial;
    this->auxPin = auxPin;
    this->bpsRate = bpsRate;
}

Ebyte_E34::Ebyte_E34(HardwareSerial * serial, byte auxPin, byte m0Pin, byte m1Pin, UART_BPS_RATE bpsRate) {
    this->hs = serial;
    this->auxPin = auxPin;
    this->m0Pin = m0Pin;
    this->m1Pin = m1Pin;
    this->bpsRate = bpsRate;
}

Ebyte_E34::Ebyte_E34(byte txE32pin, byte rxE32pin,
                     HardwareSerial * serial,
                     UART_BPS_RATE bpsRate, uint32_t serialConfig) {
    this->txE32pin = txE32pin;
    this->rxE32pin = rxE32pin;

    this->hs = serial;
    this->bpsRate = bpsRate;
    this->serialConfig = serialConfig;
}

Ebyte_E34::Ebyte_E34(byte txE32pin, byte rxE32pin,
                     HardwareSerial * serial, byte auxPin,
                     UART_BPS_RATE bpsRate, uint32_t serialConfig) {
    this->txE32pin = txE32pin;
    this->rxE32pin = rxE32pin;

    this->hs = serial;
    this->auxPin = auxPin;
    this->bpsRate = bpsRate;
    this->serialConfig = serialConfig;
}

Ebyte_E34::Ebyte_E34(byte txE32pin, byte rxE32pin,
                     HardwareSerial * serial, byte auxPin, byte m0Pin, byte m1Pin,
                     UART_BPS_RATE bpsRate, uint32_t serialConfig) {
    this->txE32pin = txE32pin;
    this->rxE32pin = rxE32pin;

    this->hs = serial;
    this->auxPin = auxPin;
    this->m0Pin = m0Pin;
    this->m1Pin = m1Pin;
    this->bpsRate = bpsRate;
    this->serialConfig = serialConfig;
}


/**
 * @brief Begin the operation. Should be in the setup().
 *
 */
bool Ebyte_E34::begin() {
    DEBUG_PRINT("RX MIC ---> ");
    DEBUG_PRINTLN(this->txE32pin);
    DEBUG_PRINT("TX MIC ---> ");
    DEBUG_PRINTLN(this->rxE32pin);
    DEBUG_PRINT("AUX ---> ");
    DEBUG_PRINTLN(this->auxPin);
    DEBUG_PRINT("M0 ---> ");
    DEBUG_PRINTLN(this->m0Pin);
    DEBUG_PRINT("M1 ---> ");
    DEBUG_PRINTLN(this->m1Pin);

    if (this->auxPin != -1) {
        pinMode(this->auxPin, INPUT);
        DEBUG_PRINTLN("Init AUX pin!");
    }
    if (this->m0Pin != -1) {
        pinMode(this->m0Pin, OUTPUT);
        DEBUG_PRINTLN("Init M0 pin!");
        digitalWrite(this->m0Pin, HIGH);
    }
    if (this->m1Pin != -1) {
        pinMode(this->m1Pin, OUTPUT);
        DEBUG_PRINTLN("Init M1 pin!");
        digitalWrite(this->m1Pin, HIGH);
    }

    DEBUG_PRINTLN("Begin ex");
    if (this->hs) {
        DEBUG_PRINTLN("Begin Hardware Serial");

        if (this->txE32pin != -1 && this->rxE32pin != -1) {
            this->serialDef.begin(*this->hs, this->bpsRate, this->serialConfig, this->txE32pin, this->rxE32pin);
        }
        else {
            this->serialDef.begin(*this->hs, this->bpsRate, this->serialConfig);
        }

        while (!this->hs) {
            vTaskDelay(1);  // wait for serial port to connect. Needed for native USB
        }
    }

    this->serialDef.stream->setTimeout(1000);  // Timeout data in the buffer, then send.
    Status status = setMode(MODE_0_FIXED);
    return status == E32_SUCCESS;
}


/**
 * Utility method to wait until module does not transmit.
 * The timeout is provided to avoid an infinite loop
 */
Status Ebyte_E34::waitCompleteResponse(unsigned long timeout, unsigned int waitNoAux) {
    Status result = E32_SUCCESS;
    unsigned long t_prev = millis();

    // if AUX pin was supplied and look for HIGH state
    // note you can omit using AUX if no pins are available, but you will have to use delay() to let module finish
    if (this->auxPin != -1) {
        while (digitalRead(this->auxPin) == LOW) {
            unsigned long t = millis();  // It will be overflow about every 50 days.

            if (((t >= t_prev) && ((t - t_prev) > timeout)  // Normal count up
                ) ||
                ((t < t_prev)  && (((unsigned long)(0-1) - (t_prev-t) + 1) > timeout)  // Overflow
                )) {
                result = ERR_E32_TIMEOUT;
                DEBUG_PRINTLN("Timeout error!");
                return result;
            }
        }
        DEBUG_PRINTLN("AUX HIGH!");
    }
    else {
        // If you can't use aux pin, use 4K7 pullup with Arduino.
        // You may need to adjust this value if transmissions fail.
        this->managedDelay(waitNoAux);
        DEBUG_PRINTLN(F("Wait no AUX pin!"));
    }

    // As per data sheet, control after aux goes high is 2ms; so delay for at least that long
    this->managedDelay(2);
    DEBUG_PRINTLN(F("Complete!"));
    return result;
}


/**
 * Delay() in a library is not a good idea as it can stop interrupts.
 * Just poll internal time until timeout is reached.
 */
void Ebyte_E34::managedDelay(unsigned long timeout) {
    unsigned long t_prev = millis();  // It will be overflow about every 50 days.
    unsigned long t = t_prev;

    while (1) {
        if (((t >= t_prev) && ((t - t_prev) > timeout)  // Normal count up
            ) ||
            ((t < t_prev)  && (((unsigned long)(0-1) - (t_prev-t) + 1) > timeout)  // Overflow
            )) {
            break;
        }

        taskYIELD();
    }
}


/**
 * Method to indicate availability & to clear the buffer
 */
int Ebyte_E34::available() {
    return this->serialDef.stream->available();
}

void Ebyte_E34::flush() {
    this->serialDef.stream->flush();
}

void Ebyte_E34::cleanUARTBuffer() {
    while (this->available()) {
        this->serialDef.stream->read();
    }
}


/*

Method to send a chunk of data provided data is in a struct--my personal favorite as you
need not parse or worry about sprintf() inability to handle floats

TTP: put your structure definition into a .h file and include in both the sender and reciever
sketches

NOTE: of your sender and receiver MCU's are different (Teensy and Arduino) caution on the data
types each handle ints floats differently

*/

Status Ebyte_E34::sendStruct(void * structureManaged, uint16_t size_) {
    if (size_ > MAX_SIZE_TX_PACKET + 2) {
        return ERR_E32_PACKET_TOO_BIG;
    }

    Status result = E32_SUCCESS;

    uint8_t len = this->serialDef.stream->write((uint8_t *)structureManaged, size_);
    if (len != size_) {
        DEBUG_PRINT(F("Send... len:"))
        DEBUG_PRINT(len);
        DEBUG_PRINT(F(" size:"))
        DEBUG_PRINT(size_);
        if (len == 0) {
            result = ERR_E32_NO_RESPONSE_FROM_DEVICE;
        }
        else {
            result = ERR_E32_DATA_SIZE_NOT_MATCH;
        }
    }
    if (result != E32_SUCCESS) return result;

    result = this->waitCompleteResponse(1000);
    if (result != E32_SUCCESS) return result;
    DEBUG_PRINT(F("Clear buffer..."))
    this->cleanUARTBuffer();

    DEBUG_PRINTLN(F("ok!"))

    return result;
}

/**
 * Method to get a chunk of data provided data is in a struct--my personal favorite as you
 * need not parse or worry about sprintf() inability to handle floats
 *
 * TTP: put your structure definition into a .h file and include in both the sender and reciever
 * sketches
 *
 * NOTE: of your sender and receiver MCU's are different (Teensy and Arduino) caution on the data
 * types each handle ints floats differently
 */

Status Ebyte_E34::receiveStruct(void * structureManaged, uint16_t size_) {
    Status result = E32_SUCCESS;

    uint8_t len = this->serialDef.stream->readBytes((uint8_t *)structureManaged, size_);

    DEBUG_PRINT("Available buffer: ");
    DEBUG_PRINT(len);
    DEBUG_PRINT(" structure size: ");
    DEBUG_PRINTLN(size_);

    if (len != size_) {
        if (len == 0) {
            result = ERR_E32_NO_RESPONSE_FROM_DEVICE;
        }
        else {
            result = ERR_E32_DATA_SIZE_NOT_MATCH;
        }
    }
    if (result != E32_SUCCESS) return result;

    result = this->waitCompleteResponse(1000);
    if (result != E32_SUCCESS) return result;

    return result;
}

/**
 * Method to set the mode (program, normal, etc.)
 */

Status Ebyte_E34::setMode(MODE_TYPE mode) {

    // data sheet claims module needs some extra time after mode setting (2ms)
    // most of my projects uses 10 ms, but 40ms is safer

    this->managedDelay(40);

    if (this->m0Pin == -1 && this->m1Pin == -1) {
        DEBUG_PRINTLN(F("The M0 and M1 pins is not set, this mean that you are connect directly the pins as you need!"))
    }
    else {
        switch (mode) {
        case MODE_0_FIXED:
            // Mode 0 | normal operation
            digitalWrite(this->m0Pin, LOW);
            digitalWrite(this->m1Pin, LOW);
            DEBUG_PRINTLN("MODE FIXED FREQ!");
            break;
        case MODE_1_HOPPING:
            digitalWrite(this->m0Pin, HIGH);
            digitalWrite(this->m1Pin, LOW);
            DEBUG_PRINTLN("MODE WAKE UP!");
            break;
        case MODE_2_POWER_SAVING:
            digitalWrite(this->m0Pin, LOW);
            digitalWrite(this->m1Pin, HIGH);
            DEBUG_PRINTLN("MODE POWER SAVING!");
            break;
        case MODE_3_SLEEP:
            // Mode 3 | Setting operation
            digitalWrite(this->m0Pin, HIGH);
            digitalWrite(this->m1Pin, HIGH);
            DEBUG_PRINTLN("MODE PROGRAM/SLEEP!");
            break;
        default: return ERR_E32_INVALID_PARAM;
        }
    }
    // data sheet says 2ms later control is returned, let's give just a bit more time
    // these modules can take time to activate pins
    this->managedDelay(40);

    // wait until aux pin goes back low
    Status res = this->waitCompleteResponse(1000);

    if (res == E32_SUCCESS) {
        this->mode = mode;
    }

    return res;
}

MODE_TYPE Ebyte_E34::getMode() { return this->mode; }

void Ebyte_E34::writeProgramCommand(PROGRAM_COMMAND cmd) {
    uint8_t CMD[3] = {cmd, cmd, cmd};
    // uint8_t size =
    this->serialDef.stream->write(CMD, 3);
    this->managedDelay(50);  // need ti check
}

ResponseStructContainer Ebyte_E34::getConfiguration() {
    ResponseStructContainer rc;

    rc.status.code = checkUARTConfiguration(MODE_3_PROGRAM);
    if (rc.status.code != E32_SUCCESS) return rc;

    MODE_TYPE prevMode = this->mode;

    rc.status.code = this->setMode(MODE_3_PROGRAM);
    if (rc.status.code != E32_SUCCESS) return rc;

    this->writeProgramCommand(READ_CONFIGURATION);

    rc.data        = malloc(sizeof(Configuration));
    rc.status.code = this->receiveStruct((uint8_t *)rc.data, sizeof(Configuration));

#ifdef EBYTE_DEBUG
    this->printParameters((Configuration *)rc.data);
#endif

    if (rc.status.code != E32_SUCCESS) {
        this->setMode(prevMode);
        return rc;
    }

    DEBUG_PRINTLN("----------------------------------------");
    DEBUG_PRINT(F("HEAD BIN INSIDE: "));
    DEBUG_PRINT(((Configuration *)rc.data)->HEAD, BIN);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(((Configuration *)rc.data)->HEAD, DEC);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN(((Configuration *)rc.data)->HEAD, HEX);
    DEBUG_PRINTLN("----------------------------------------");

    rc.status.code = this->setMode(prevMode);
    if (rc.status.code != E32_SUCCESS) return rc;

    //	this->printParameters(*configuration);

    if (0xC0 != ((Configuration *)rc.data)->HEAD && 0xC2 != ((Configuration *)rc.data)->HEAD) {
        rc.status.code = ERR_E32_HEAD_NOT_RECOGNIZED;
    }

    //	rc.data = configuration;
    return rc;
}

RESPONSE_STATUS Ebyte_E34::checkUARTConfiguration(MODE_TYPE mode) {
    if (mode == MODE_3_PROGRAM && this->bpsRate != UART_BPS_RATE_9600) {
        return ERR_E32_WRONG_UART_CONFIG;
    }
    return E32_SUCCESS;
}

ResponseStatus Ebyte_E34::setConfiguration(Configuration configuration, PROGRAM_COMMAND saveType) {
    ResponseStatus rc;

    rc.code = checkUARTConfiguration(MODE_3_PROGRAM);
    if (rc.code != E32_SUCCESS) return rc;

    MODE_TYPE prevMode = this->mode;

    rc.code = this->setMode(MODE_3_PROGRAM);
    if (rc.code != E32_SUCCESS) return rc;

    this->writeProgramCommand(READ_CONFIGURATION);

    configuration.HEAD = saveType;

    rc.code = this->sendStruct((uint8_t *)&configuration, sizeof(Configuration));
    if (rc.code != E32_SUCCESS) {
        this->setMode(prevMode);
        return rc;
    }

    DEBUG_PRINTLN("----------------------------------------");
    DEBUG_PRINT(F("HEAD BIN INSIDE: "));
    DEBUG_PRINT(configuration.HEAD, BIN);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(configuration.HEAD, DEC);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN(configuration.HEAD, HEX);
    DEBUG_PRINTLN("----------------------------------------");

    rc.code = this->setMode(prevMode);
    if (rc.code != E32_SUCCESS) return rc;

    //	this->printParameters(*configuration);

    if (0xC0 != configuration.HEAD && 0xC2 != configuration.HEAD) {
        rc.code = ERR_E32_HEAD_NOT_RECOGNIZED;
    }

    return rc;
}

ResponseStructContainer Ebyte_E34::getModuleInformation() {
    ResponseStructContainer rc;

    rc.status.code = checkUARTConfiguration(MODE_3_PROGRAM);
    if (rc.status.code != E32_SUCCESS) return rc;

    MODE_TYPE prevMode = this->mode;

    rc.status.code = this->setMode(MODE_3_PROGRAM);
    if (rc.status.code != E32_SUCCESS) return rc;

    this->writeProgramCommand(READ_MODULE_VERSION);

    struct ModuleInformation * moduleInformation = (ModuleInformation *)malloc(sizeof(ModuleInformation));
    rc.status.code = this->receiveStruct((uint8_t *)moduleInformation, sizeof(ModuleInformation));
    if (rc.status.code != E32_SUCCESS) {
        this->setMode(prevMode);
        return rc;
    }

    rc.status.code = this->setMode(prevMode);
    if (rc.status.code != E32_SUCCESS) return rc;

    //	this->printParameters(*configuration);

    if (0xC3 != moduleInformation->HEAD) {
        rc.status.code = ERR_E32_HEAD_NOT_RECOGNIZED;
    }

    DEBUG_PRINTLN("----------------------------------------");
    DEBUG_PRINT(F("HEAD BIN INSIDE: "));
    DEBUG_PRINT(moduleInformation->HEAD, BIN);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(moduleInformation->HEAD, DEC);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN(moduleInformation->HEAD, HEX);

    DEBUG_PRINT(F("Freq.: "));
    DEBUG_PRINTLN(moduleInformation->frequency, HEX);
    DEBUG_PRINT(F("Version  : "));
    DEBUG_PRINTLN(moduleInformation->version, HEX);
    DEBUG_PRINT(F("Features : "));
    DEBUG_PRINTLN(moduleInformation->features, HEX);
    DEBUG_PRINTLN("----------------------------------------");

    rc.data = moduleInformation;  // malloc(sizeof (moduleInformation));

    return rc;
}

ResponseStatus Ebyte_E34::resetModule() {
    ResponseStatus status;

    status.code = checkUARTConfiguration(MODE_3_PROGRAM);
    if (status.code != E32_SUCCESS) return status;

    MODE_TYPE prevMode = this->mode;

    status.code = this->setMode(MODE_3_PROGRAM);
    if (status.code != E32_SUCCESS) return status;

    this->writeProgramCommand(WRITE_RESET_MODULE);

    status.code = this->waitCompleteResponse(1000);
    if (status.code != E32_SUCCESS) {
        this->setMode(prevMode);
        return status;
    }

    status.code = this->setMode(prevMode);
    if (status.code != E32_SUCCESS) return status;

    return status;
}

ResponseContainer Ebyte_E34::receiveMessage() {
    ResponseContainer rc;
    rc.status.code = E32_SUCCESS;
    rc.data        = this->serialDef.stream->readString();
    this->cleanUARTBuffer();
    if (rc.status.code != E32_SUCCESS) {
        return rc;
    }

    //	rc.data = message; // malloc(sizeof (moduleInformation));

    return rc;
}

ResponseContainer Ebyte_E34::receiveMessageUntil(char delimiter) {
    ResponseContainer rc;
    rc.status.code = E32_SUCCESS;
    rc.data        = this->serialDef.stream->readStringUntil(delimiter);
    //	this->cleanUARTBuffer();
    if (rc.status.code != E32_SUCCESS) {
        return rc;
    }

    //	rc.data = message; // malloc(sizeof (moduleInformation));

    return rc;
}

ResponseStructContainer Ebyte_E34::receiveMessage(const uint8_t size) {
    ResponseStructContainer rc;

    rc.data        = malloc(size);
    rc.status.code = this->receiveStruct((uint8_t *)rc.data, size);
    this->cleanUARTBuffer();
    if (rc.status.code != E32_SUCCESS) {
        return rc;
    }

    return rc;
}

ResponseStatus Ebyte_E34::sendMessage(const void * message, const uint8_t size) {
    ResponseStatus status;
    status.code = this->sendStruct((uint8_t *)message, size);
    if (status.code != E32_SUCCESS) return status;

    return status;
}

ResponseStatus Ebyte_E34::sendMessage(const String message) {
    DEBUG_PRINT(F("Send message: "));
    DEBUG_PRINT(message);
    byte size = message.length();  // sizeof(message.c_str())+1;
    DEBUG_PRINT(F(" size: "));
    DEBUG_PRINTLN(size);
    char messageFixed[size];
    memcpy(messageFixed, message.c_str(), size);

    ResponseStatus status;
    status.code = this->sendStruct((uint8_t *)&messageFixed, size);
    if (status.code != E32_SUCCESS) return status;

    return status;
}

ResponseStatus Ebyte_E34::sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const String message) {
    byte size = message.length();
    char messageFixed[size];
    memcpy(messageFixed, message.c_str(), size);
    return this->sendFixedMessage(ADDH, ADDL, CHAN, (uint8_t *)messageFixed, size);
}

ResponseStatus Ebyte_E34::sendBroadcastFixedMessage(byte CHAN, const String message) {
    return this->sendFixedMessage(BROADCAST_ADDRESS, BROADCAST_ADDRESS, CHAN, message);
}

typedef struct fixedStransmission {
    byte          ADDH = 0;
    byte          ADDL = 0;
    byte          CHAN = 0;
    unsigned char message[];
} FixedStransmission;

FixedStransmission * init_stack(int m) {
    FixedStransmission * st = (FixedStransmission *)malloc(sizeof(FixedStransmission) + m * sizeof(int));
    return st;
}

ResponseStatus Ebyte_E34::sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const void * message, const uint8_t size) {
    FixedStransmission * fixedStransmission = init_stack(size);

    fixedStransmission->ADDH = ADDH;
    fixedStransmission->ADDL = ADDL;
    fixedStransmission->CHAN = CHAN;

    memcpy(fixedStransmission->message, (unsigned char *)message, size);

    ResponseStatus status;
    status.code = this->sendStruct((uint8_t *)fixedStransmission, size + 3);

    free(fixedStransmission);

    if (status.code != E32_SUCCESS) return status;

    return status;
}

ResponseStatus Ebyte_E34::sendBroadcastFixedMessage(byte CHAN, const void * message, const uint8_t size) {
    return this->sendFixedMessage(0xFF, 0xFF, CHAN, message, size);
}

ResponseContainer Ebyte_E34::receiveInitialMessage(uint8_t size) {
    ResponseContainer rc;
    rc.status.code = E32_SUCCESS;
    char    buff[size];
    uint8_t len = this->serialDef.stream->readBytes(buff, size);
    if (len != size) {
        if (len == 0) {
            rc.status.code = ERR_E32_NO_RESPONSE_FROM_DEVICE;
        }
        else {
            rc.status.code = ERR_E32_DATA_SIZE_NOT_MATCH;
        }
        return rc;
    }

    rc.data = buff;

    return rc;
}

#ifdef EBYTE_DEBUG
void Ebyte_E34::printParameters(struct Configuration * configuration) {
    DEBUG_PRINTLN("----------------------------------------");

    DEBUG_PRINT(F("HEAD : "));
    DEBUG_PRINT(configuration->HEAD, BIN);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(configuration->HEAD, DEC);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN(configuration->HEAD, HEX);
    DEBUG_PRINTLN(F(" "));
    DEBUG_PRINT(F("AddH : "));
    DEBUG_PRINTLN(configuration->ADDH, DEC);
    DEBUG_PRINT(F("AddL : "));
    DEBUG_PRINTLN(configuration->ADDL, DEC);
    DEBUG_PRINT(F("Chan : "));
    DEBUG_PRINT(configuration->CHAN, DEC);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->getChannelDescription());
    DEBUG_PRINTLN(F(" "));
    DEBUG_PRINT(F("SpeedParityBit     : "));
    DEBUG_PRINT(configuration->SPED.uartParity, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->SPED.getUARTParityDescription());
    DEBUG_PRINT(F("SpeedUARTDatte  : "));
    DEBUG_PRINT(configuration->SPED.uartBaudRate, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->SPED.getUARTBaudRate());
    DEBUG_PRINT(F("SpeedAirDataRate   : "));
    DEBUG_PRINT(configuration->SPED.airDataRate, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->SPED.getAirDataRate());

    DEBUG_PRINT(F("OptionTrans        : "));
    DEBUG_PRINT(configuration->OPTION.fixedTransmission, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->OPTION.getFixedTransmissionDescription());
    DEBUG_PRINT(F("OptionPullup       : "));
    DEBUG_PRINT(configuration->OPTION.ioDriveMode, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->OPTION.getIODroveModeDescription());
    DEBUG_PRINT(F("OptionWakeup       : "));
    DEBUG_PRINT(configuration->OPTION.wirelessWakeupTime, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->OPTION.getWirelessWakeUPTimeDescription());
    DEBUG_PRINT(F("OptionFEC          : "));
    DEBUG_PRINT(configuration->OPTION.fec, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->OPTION.getFECDescription());
    DEBUG_PRINT(F("OptionPower        : "));
    DEBUG_PRINT(configuration->OPTION.transmissionPower, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->OPTION.getTransmissionPowerDescription());

    DEBUG_PRINTLN("----------------------------------------");
}
#endif
