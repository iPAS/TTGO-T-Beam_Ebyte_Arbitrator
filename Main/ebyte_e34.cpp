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
 */
Ebyte_E34::Ebyte_E34(HardwareSerial * serial, byte auxPin, byte m0Pin, byte m1Pin, byte rxPin, byte txPin) {
    this->hs = serial;

    this->auxPin = auxPin;
    this->m0Pin = m0Pin;
    this->m1Pin = m1Pin;

    this->rxPin = rxPin;
    this->txPin = txPin;
}


/**
 * @brief Begin the operation. Should be in the setup().
 */
bool Ebyte_E34::begin() {
    DEBUG_PRINTLN("[EBYTE] begin");

    DEBUG_PRINT("uC RX to TX ---> "); DEBUG_PRINTLN(this->txPin);
    DEBUG_PRINT("uC TX to RX ---> "); DEBUG_PRINTLN(this->rxPin);
    DEBUG_PRINT("AUX ---> ");         DEBUG_PRINTLN(this->auxPin);
    DEBUG_PRINT("M0 ---> ");          DEBUG_PRINTLN(this->m0Pin);
    DEBUG_PRINT("M1 ---> ");          DEBUG_PRINTLN(this->m1Pin);

    if (this->auxPin != -1) {
        pinMode(this->auxPin, INPUT);
        DEBUG_PRINTLN("Init AUX pin!");
    }
    if (this->m0Pin != -1) {
        pinMode(this->m0Pin, OUTPUT);
        digitalWrite(this->m0Pin, HIGH);
        DEBUG_PRINTLN("Init M0 pin!");
    }
    if (this->m1Pin != -1) {
        pinMode(this->m1Pin, OUTPUT);
        digitalWrite(this->m1Pin, HIGH);
        DEBUG_PRINTLN("Init M1 pin!");
    }

    if (this->hs) {
        if (this->txPin != -1 && this->rxPin != -1) {
            this->serialDef.begin(*this->hs, this->bpsRate, this->serialConfig,
                                  this->txPin,  // To RX of uC
                                  this->rxPin   // To TX of uC
                                  );
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
    return status == E34_SUCCESS;
}


/**
 * @brief Get MODE
 *
 * @return MODE_TYPE
 */
MODE_TYPE Ebyte_E34::getMode() {
    return this->mode;
}


/**
 * @brief Set MODE
 *
 * @param mode
 * @return Status
 */
Status Ebyte_E34::setMode(MODE_TYPE mode) {
    // Datasheet claims module needs some extra time after mode setting (2ms).
    // However, most of my projects uses 10 ms, but 40ms is safer.
    this->managedDelay(EBYTE_EXTRA_WAIT);

    if (this->m0Pin == -1 && this->m1Pin == -1) {
        DEBUG_PRINTLN(F(
            "The M0 and M1 pins is not set,"
            " this mean that you are connect directly the pins as you need!"));
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
                DEBUG_PRINTLN("MODE HOPPING FREQ!");
                break;
            case MODE_2_RESERVED:
                digitalWrite(this->m0Pin, LOW);
                digitalWrite(this->m1Pin, HIGH);
                DEBUG_PRINTLN("MODE RESERVATION!");
                break;
            case MODE_3_SLEEP:
                // Mode 3 | Setting operation
                digitalWrite(this->m0Pin, HIGH);
                digitalWrite(this->m1Pin, HIGH);
                DEBUG_PRINTLN("MODE PROGRAM/SLEEP!");
                break;
            default: return ERR_E34_INVALID_PARAM;
        }
    }

    // The datasheet says after 2ms later, control is returned.
    // Let's give just a bit more time for this module to be active.
    this->managedDelay(EBYTE_EXTRA_WAIT);

    // Wait until AUX pin goes back to low.
    Status res = this->waitCompleteResponse(1000);

    if (res == E34_SUCCESS) {
        this->mode = mode;
    }

    return res;
}


/**
 * @brief Check whether timeout or not
 */
static bool is_timeout(unsigned long t, unsigned long t_prev, unsigned long timeout) {
    return (((t >= t_prev) && ((t - t_prev) >= timeout)  // Normal count up
            ) ||
            ((t < t_prev)  && (((unsigned long)(0-1) - (t_prev-t) + 1) >= timeout)  // Overflow
            ))? true : false;
}


/**
 * Delay() in a library is not a good idea as it can stop interrupts.
 * Just poll internal time until timeout is reached.
 */
void Ebyte_E34::managedDelay(unsigned long timeout) {
    unsigned long t_prev = millis();  // It will be overflow about every 50 days.

    while (1) {
        unsigned long t = millis();

        if (is_timeout(t, t_prev, timeout)) {
            break;
        }

        taskYIELD();
    }
}


/**
 * Utility method to wait until module does not transmit.
 * The timeout is provided to avoid an infinite loop
 */
Status Ebyte_E34::waitCompleteResponse(unsigned long timeout, unsigned int waitNoAux) {
    Status result = E34_SUCCESS;
    unsigned long t_prev = millis();

    // If AUX pin was supplied, and look for HIGH state.
    // XXX: you can omit using AUX if no pins are available, but you will have to use delay() to let module finish
    if (this->auxPin != -1) {
        while (digitalRead(this->auxPin) == LOW) {
            unsigned long t = millis();  // It will be overflow about every 50 days.

            if (is_timeout(t, t_prev, timeout)) {
                result = ERR_E34_TIMEOUT;
                DEBUG_PRINTLN("Wait response: timeout error!");
                return result;
            }
        }
        DEBUG_PRINTLN("AUX HIGH!");
    }
    else {
        // If you can't use aux pin, use 4K7 pullup with Arduino.
        // You may need to adjust this value if transmissions fail.
        this->managedDelay(waitNoAux);
        DEBUG_PRINTLN(F("Wait response: no AUX pin -- just wait.."));
    }

    // As per data sheet, control after aux goes high is 2ms; so delay for at least that long
    this->managedDelay(EBYTE_EXTRA_WAIT);
    DEBUG_PRINTLN(F("Wait response: complete!"));
    return result;
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


/**
 * @brief Write command
 *
 * @param cmd
 */
void Ebyte_E34::writeProgramCommand(PROGRAM_COMMAND cmd) {
    uint8_t CMD[3] = {cmd, cmd, cmd};
    // uint8_t size =
    this->serialDef.stream->write(CMD, 3);
    this->managedDelay(EBYTE_EXTRA_WAIT);
}


/**
 * @brief Get configuration
 *
 * @return ResponseStructContainer
 */
ResponseStructContainer Ebyte_E34::getConfiguration() {
    ResponseStructContainer rc;

    rc.status.code = checkUARTConfiguration(MODE_3_PROGRAM);
    if (rc.status.code != E34_SUCCESS) return rc;

    MODE_TYPE prevMode = this->mode;

    rc.status.code = this->setMode(MODE_3_PROGRAM);
    if (rc.status.code != E34_SUCCESS) return rc;

    this->writeProgramCommand(READ_CONFIGURATION);

    rc.data        = malloc(sizeof(Configuration));
    rc.status.code = this->receiveStruct((uint8_t *)rc.data, sizeof(Configuration));

    if (rc.status.code != E34_SUCCESS) {
        this->setMode(prevMode);
        return rc;
    }

    #ifdef EBYTE_DEBUG
    this->printHead(((Configuration *)rc.data)->HEAD);
    #endif

    rc.status.code = this->setMode(prevMode);
    if (rc.status.code != E34_SUCCESS) return rc;

    if ((0xC0 != ((Configuration *)rc.data)->HEAD
        ) &&
        (0xC2 != ((Configuration *)rc.data)->HEAD
        )) {
        rc.status.code = ERR_E34_HEAD_NOT_RECOGNIZED;
    }

    return rc;
}

ResponseStatus Ebyte_E34::setConfiguration(Configuration configuration, PROGRAM_COMMAND saveType) {
    ResponseStatus rc;

    rc.code = checkUARTConfiguration(MODE_3_PROGRAM);
    if (rc.code != E34_SUCCESS) return rc;

    MODE_TYPE prevMode = this->mode;

    rc.code = this->setMode(MODE_3_PROGRAM);
    if (rc.code != E34_SUCCESS) return rc;

    this->writeProgramCommand(READ_CONFIGURATION);

    configuration.HEAD = saveType;

    rc.code = this->sendStruct((uint8_t *)&configuration, sizeof(Configuration));
    if (rc.code != E34_SUCCESS) {
        this->setMode(prevMode);
        return rc;
    }

    #ifdef EBYTE_DEBUG
    this->printHead(configuration.HEAD);
    #endif

    rc.code = this->setMode(prevMode);
    if (rc.code != E34_SUCCESS) return rc;

    if ((0xC0 != configuration.HEAD) && (0xC2 != configuration.HEAD)) {
        rc.code = ERR_E34_HEAD_NOT_RECOGNIZED;
    }

    return rc;
}

ResponseStructContainer Ebyte_E34::getModuleInformation() {
    ResponseStructContainer rc;

    rc.status.code = checkUARTConfiguration(MODE_3_PROGRAM);
    if (rc.status.code != E34_SUCCESS) return rc;

    MODE_TYPE prevMode = this->mode;

    rc.status.code = this->setMode(MODE_3_PROGRAM);
    if (rc.status.code != E34_SUCCESS) return rc;

    this->writeProgramCommand(READ_MODULE_VERSION);

    struct ModuleInformation * moduleInformation = (ModuleInformation *)malloc(sizeof(ModuleInformation));
    rc.status.code = this->receiveStruct((uint8_t *)moduleInformation, sizeof(ModuleInformation));
    if (rc.status.code != E34_SUCCESS) {
        this->setMode(prevMode);
        return rc;
    }

    rc.status.code = this->setMode(prevMode);
    if (rc.status.code != E34_SUCCESS) return rc;

    if (0xC3 != moduleInformation->HEAD) {
        rc.status.code = ERR_E34_HEAD_NOT_RECOGNIZED;
    }

    #ifdef EBYTE_DEBUG
    this->printHead(moduleInformation->HEAD);
    #endif
    DEBUG_PRINT(F("Freq.: "));
    DEBUG_PRINTLN(moduleInformation->frequency, HEX);
    DEBUG_PRINT(F("Version  : "));
    DEBUG_PRINTLN(moduleInformation->version, HEX);
    DEBUG_PRINT(F("Features : "));
    DEBUG_PRINTLN(moduleInformation->features, HEX);

    rc.data = moduleInformation;  // malloc(sizeof (moduleInformation));
    return rc;
}

ResponseStatus Ebyte_E34::resetModule() {
    ResponseStatus status;

    status.code = checkUARTConfiguration(MODE_3_PROGRAM);
    if (status.code != E34_SUCCESS) return status;

    MODE_TYPE prevMode = this->mode;

    status.code = this->setMode(MODE_3_PROGRAM);
    if (status.code != E34_SUCCESS) return status;

    this->writeProgramCommand(WRITE_RESET_MODULE);

    status.code = this->waitCompleteResponse(1000);
    if (status.code != E34_SUCCESS) {
        this->setMode(prevMode);
        return status;
    }

    status.code = this->setMode(prevMode);
    return status;
}


/**
 * @brief Check UART configuration for mode
 *
 * @param mode
 * @return RESPONSE_STATUS
 */
RESPONSE_STATUS Ebyte_E34::checkUARTConfiguration(MODE_TYPE mode) {
    if (mode == MODE_3_PROGRAM && this->bpsRate != 9600) {
        return ERR_E34_WRONG_UART_CONFIG;
    }
    return E34_SUCCESS;
}


/**
 * Method to send or receive a chunk of data provided in the struct.
 * They are my personal favorites.
 * You need not parse or worry about sprintf() inability to handle floats.
 *
 * Put your structure definition into a .h file and include in both the sender and reciever sketches.
 */
Status Ebyte_E34::sendStruct(const void * structureManaged, uint16_t size_of_st) {
    if (size_of_st > MAX_SIZE_TX_PACKET + 2) {
        return ERR_E34_PACKET_TOO_BIG;
    }

    Status result = E34_SUCCESS;
    uint8_t len = this->serialDef.stream->write((uint8_t *)structureManaged, size_of_st);

    DEBUG_PRINT(F("Send struct len:"));
    DEBUG_PRINT(len);
    DEBUG_PRINT(F(" size:"))
    DEBUG_PRINTLN(size_of_st);

    if (len != size_of_st) {
        return (len == 0)? ERR_E34_NO_RESPONSE_FROM_DEVICE : ERR_E34_DATA_SIZE_NOT_MATCH;
    }
    result = this->waitCompleteResponse(1000);
    if (result != E34_SUCCESS) return result;

    DEBUG_PRINTLN(F("Clear Tx buf..."))
    this->cleanUARTBuffer();

    return result;
}

Status Ebyte_E34::receiveStruct(void * structureManaged, uint16_t size_of_st) {
    Status result = E34_SUCCESS;
    uint8_t len = this->serialDef.stream->readBytes((uint8_t *)structureManaged, size_of_st);

    DEBUG_PRINT(F("Recv struct len:"));
    DEBUG_PRINT(len);
    DEBUG_PRINT(F(" size:"));
    DEBUG_PRINTLN(size_of_st);

    if (len != size_of_st) {
        return (len == 0)? ERR_E34_NO_RESPONSE_FROM_DEVICE : ERR_E34_DATA_SIZE_NOT_MATCH;
    }
    result = this->waitCompleteResponse(1000);
    if (result != E34_SUCCESS) return result;

    return result;
}


/**
 * @brief Receiving
 *
 */

ResponseContainer Ebyte_E34::receiveMessage() {
    ResponseContainer rc;
    rc.status.code = E34_SUCCESS;
    rc.data        = this->serialDef.stream->readString();
    this->cleanUARTBuffer();
    if (rc.status.code != E34_SUCCESS) {
        return rc;
    }

    return rc;
}

ResponseContainer Ebyte_E34::receiveMessageUntil(char delimiter) {
    ResponseContainer rc;
    rc.status.code = E34_SUCCESS;
    rc.data        = this->serialDef.stream->readStringUntil(delimiter);
    //	this->cleanUARTBuffer();
    if (rc.status.code != E34_SUCCESS) {
        return rc;
    }

    return rc;
}

ResponseStructContainer Ebyte_E34::receiveMessage(const uint8_t size) {
    ResponseStructContainer rc;
    rc.data        = malloc(size);
    rc.status.code = this->receiveStruct((uint8_t *)rc.data, size);
    this->cleanUARTBuffer();
    if (rc.status.code != E34_SUCCESS) {
        return rc;
    }

    return rc;
}

ResponseContainer Ebyte_E34::receiveInitialMessage(uint8_t size) {
    ResponseContainer rc;
    rc.status.code = E34_SUCCESS;
    char    buff[size];
    uint8_t len = this->serialDef.stream->readBytes(buff, size);
    if (len != size) {
        if (len == 0) {
            rc.status.code = ERR_E34_NO_RESPONSE_FROM_DEVICE;
        }
        else {
            rc.status.code = ERR_E34_DATA_SIZE_NOT_MATCH;
        }
        return rc;
    }

    rc.data = buff;

    return rc;
}


/**
 * @brief Sending
 */

ResponseStatus Ebyte_E34::sendMessage(const String message) {
    return this->sendMessage(message.c_str(), message.length());
}

ResponseStatus Ebyte_E34::sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const String message) {
    return this->sendFixedMessage(ADDH, ADDL, CHAN, message.c_str(), message.length());
}

ResponseStatus Ebyte_E34::sendBroadcastFixedMessage(byte CHAN, const String message) {
    return this->sendFixedMessage(BROADCAST_ADDRESS, BROADCAST_ADDRESS, CHAN, message);
}


ResponseStatus Ebyte_E34::sendMessage(const void * message, uint8_t size) {
    ResponseStatus status;
    status.code = this->sendStruct(message, size);
    return status;
}

ResponseStatus Ebyte_E34::sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const void * message, const uint8_t size) {
    uint8_t message_size = sizeof(* FixedStransmission::message) * size;
    uint8_t packet_size = sizeof(FixedStransmission) + message_size;  // sizeof(FixedStransmission) neglect ::message !
    FixedStransmission * fixedPacket = (FixedStransmission *)malloc(packet_size);

    fixedPacket->ADDH = ADDH;
    fixedPacket->ADDL = ADDL;
    fixedPacket->CHAN = CHAN;
    memcpy(fixedPacket->message, message, message_size);

    ResponseStatus status;
    status.code = this->sendStruct(fixedPacket, packet_size);
    free(fixedPacket);

    return status;
}

ResponseStatus Ebyte_E34::sendBroadcastFixedMessage(byte CHAN, const void * message, const uint8_t size) {
    return this->sendFixedMessage(BROADCAST_ADDRESS, BROADCAST_ADDRESS, CHAN, message, size);
}


/**
 * @brief Print debug
 *
 */

#ifdef EBYTE_DEBUG
void Ebyte_E34::printHead(byte HEAD) {
    DEBUG_PRINT(F("HEAD : "));
    DEBUG_PRINT(HEAD, BIN);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(HEAD, DEC);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN(HEAD, HEX);
}


void Ebyte_E34::printParameters(struct Configuration * configuration) {
    DEBUG_PRINTLN("\n--- Configuration ---");

    this->printHead(configuration->HEAD);
    // DEBUG_PRINT(F("HEAD : "));
    // DEBUG_PRINT(configuration->HEAD, BIN);
    // DEBUG_PRINT(" ");
    // DEBUG_PRINT(configuration->HEAD, DEC);
    // DEBUG_PRINT(" ");
    // DEBUG_PRINTLN(configuration->HEAD, HEX);
    DEBUG_PRINTLN(F(" "));
    DEBUG_PRINT(F("AddH : "));
    DEBUG_PRINTLN(configuration->ADDH, DEC);
    DEBUG_PRINT(F("AddL : "));
    DEBUG_PRINTLN(configuration->ADDL, DEC);
    DEBUG_PRINT(F("Chan : "));
    DEBUG_PRINT(configuration->CHAN, DEC);
    // DEBUG_PRINT(" -> ");
    // DEBUG_PRINTLN(configuration->desc());
    DEBUG_PRINTLN(F(" "));
    DEBUG_PRINT(F("SpeedParityBit     : "));
    DEBUG_PRINT(configuration->SPED.uartParity, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->SPED.parity_desc());
    DEBUG_PRINT(F("SpeedUARTDatte  : "));
    DEBUG_PRINT(configuration->SPED.uartBaudRate, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->SPED.baudrate_desc());
    DEBUG_PRINT(F("SpeedAirDataRate   : "));
    DEBUG_PRINT(configuration->SPED.airDataRate, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->SPED.airrate_desc());

    DEBUG_PRINT(F("OptionTrans        : "));
    DEBUG_PRINT(configuration->OPTION.fixedTransmission, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->OPTION.fixed_tx_desc());
    DEBUG_PRINT(F("OptionPullup       : "));
    DEBUG_PRINT(configuration->OPTION.ioDriveMode, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->OPTION.io_drv_desc());
    DEBUG_PRINT(F("OptionWakeup       : "));
    DEBUG_PRINT(configuration->OPTION.wirelessWakeupTime, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->OPTION.wl_wake_desc());
    DEBUG_PRINT(F("OptionFEC          : "));
    DEBUG_PRINT(configuration->OPTION.fec, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->OPTION.fec_desc());
    DEBUG_PRINT(F("OptionPower        : "));
    DEBUG_PRINT(configuration->OPTION.transmissionPower, BIN);
    DEBUG_PRINT(" -> ");
    DEBUG_PRINTLN(configuration->OPTION.txpower_desc());

    DEBUG_PRINTLN("----------------------------------------");
}
#endif
