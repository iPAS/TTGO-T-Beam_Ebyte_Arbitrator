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
    DEBUG_PRINTLN("[E34] begin");

    DEBUG_PRINT(" RX (To Ebyte TX) ---> "); DEBUG_PRINTLN(this->rxPin);
    DEBUG_PRINT(" TX (To Ebyte RX) ---> "); DEBUG_PRINTLN(this->txPin);
    DEBUG_PRINT(" AUX ---> "); DEBUG_PRINTLN(this->auxPin);
    DEBUG_PRINT(" M0 ---> "); DEBUG_PRINTLN(this->m0Pin);
    DEBUG_PRINT(" M1 ---> "); DEBUG_PRINTLN(this->m1Pin);

    if (this->auxPin != -1) {
        pinMode(this->auxPin, INPUT);
        DEBUG_PRINTLN(" Init AUX pin!");
    }
    if (this->m0Pin != -1) {
        pinMode(this->m0Pin, OUTPUT);
        digitalWrite(this->m0Pin, HIGH);
        DEBUG_PRINTLN(" Init M0 pin!");
    }
    if (this->m1Pin != -1) {
        pinMode(this->m1Pin, OUTPUT);
        digitalWrite(this->m1Pin, HIGH);
        DEBUG_PRINTLN(" Init M1 pin!");
    }

    this->changeBpsRate(this->bpsRate);

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
        DEBUG_PRINTLN(F("[E34] Pins: M0 & M1 are not set. Hard-wiring is required!"));
    }
    else {
        DEBUG_PRINT(F("[E34] Mode: "));

        switch (mode) {
            case MODE_0_FIXED:
                // Mode 0 | normal operation
                digitalWrite(this->m0Pin, LOW);
                digitalWrite(this->m1Pin, LOW);
                DEBUG_PRINTLN("FIXED FREQ!");
                break;
            case MODE_1_HOPPING:
                digitalWrite(this->m0Pin, HIGH);
                digitalWrite(this->m1Pin, LOW);
                DEBUG_PRINTLN("HOPPING FREQ!");
                break;
            case MODE_2_RESERVED:
                digitalWrite(this->m0Pin, LOW);
                digitalWrite(this->m1Pin, HIGH);
                DEBUG_PRINTLN("RESERVATION!");
                break;
            case MODE_3_SLEEP:
                // Mode 3 | Setting operation
                digitalWrite(this->m0Pin, HIGH);
                digitalWrite(this->m1Pin, HIGH);
                DEBUG_PRINTLN("PROGRAM/SLEEP!");
                break;
            default: return ERR_E34_INVALID_PARAM;
        }
    }

    // The datasheet says after 2ms later, control is returned.
    // Let's give just a bit more time for this module to be active.
    this->managedDelay(EBYTE_EXTRA_WAIT);

    // Wait until AUX pin goes back to low.
    Status status = this->waitCompleteResponse();

    if (status == E34_SUCCESS) {
        this->mode = mode;
    }

    DEBUG_PRINTLN(F("[E34] Clear Rx buf after mode change"));
    this->cleanUARTBuffer();

    return status;
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
Status Ebyte_E34::waitCompleteResponse(unsigned long timeout, unsigned long waitNoAux) {
    Status status = this->auxReady(timeout);

    if (status == ERR_E34_NOT_IMPLEMENT) {
        // If you can't use aux pin, use 4K7 pullup with Arduino.
        // You may need to adjust this value if transmissions fail.
        this->managedDelay(waitNoAux);
        DEBUG_PRINTLN(F("[E34] Wait response: no AUX pin -- just wait.."));
    }

    // As per data sheet, control after aux goes high is 2ms; so delay for at least that long
    this->managedDelay(EBYTE_EXTRA_WAIT);
    DEBUG_PRINTLN(F("[E34] Wait response: complete!"));
    return status;
}


/**
 * Method to indicate availability & to clear the buffer
 */
int Ebyte_E34::available() {
    return this->hs->available();
}

void Ebyte_E34::flush() {
    this->hs->flush();
}

void Ebyte_E34::cleanUARTBuffer() {
    while (this->available()) {
        this->hs->read();
    }
}

Status Ebyte_E34::auxReady(unsigned long timeout) {
    unsigned long t_prev = millis();

    // If AUX pin was supplied, and look for HIGH state.
    // XXX: You can omit using AUX if no pins are available, but you will have to use delay() to let module finish
    if (this->auxPin != -1) {
        while (digitalRead(this->auxPin) == LOW) {
            unsigned long t = millis();  // It will be overflow about every 50 days.

            if (is_timeout(t, t_prev, timeout)) {
                DEBUG_PRINTLN(F("[E34] Wait AUX HIGH: timeout! AUX still LOW"));
                return ERR_E34_TIMEOUT;
            }

            DEBUG_PRINTLN(F("[E34] Wait AUX HIGH.."));
            taskYIELD();
        }
        DEBUG_PRINTLN(F("[E34] AUX HIGH!"));
    }
    else {
        DEBUG_PRINTLN(F("[E34] Wait AUX HIGH: no AUX pin"));
        return ERR_E34_NOT_IMPLEMENT;
    }

    return E34_SUCCESS;
}

uint32_t Ebyte_E34::getBpsRate() {
    return this->bpsRate;
}

void Ebyte_E34::changeBpsRate(uint32_t new_bps) {
    this->hs->end();
    this->bpsRate = new_bps;

    if (this->hs) {
        this->hs->setRxBufferSize(EBYTE_UART_BUFFER_SIZE);

        if (this->txPin != -1 && this->rxPin != -1) {
            this->hs->begin(this->bpsRate, this->serialConfig,
                            this->rxPin,  // To TX of Ebyte
                            this->txPin   // To RX of Ebyte
                            );
        }
        else {
            this->hs->begin(this->bpsRate, this->serialConfig);
        }

        this->hs->setTimeout(EBYTE_UART_BUFFER_TMO);

        while (!this->hs) taskYIELD();  // wait for serial port to connect. Needed for native USB
    }

    this->hs->setTimeout(1000);  // Timeout data in the buffer, then send.
}


/**
 * @brief Write command
 *
 * @param cmd
 */
void Ebyte_E34::writeProgramCommand(PROGRAM_COMMAND cmd) {
    uint8_t CMD[3] = {cmd, cmd, cmd};
    // uint8_t size =
    this->hs->write(CMD, 3);
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
    DEBUG_PRINTLN(F("[E34] Get configuration"));
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
    ResponseStatus resp_sts;

    resp_sts.code = checkUARTConfiguration(MODE_3_PROGRAM);
    if (resp_sts.code != E34_SUCCESS) return resp_sts;

    MODE_TYPE prevMode = this->mode;

    resp_sts.code = this->setMode(MODE_3_PROGRAM);
    if (resp_sts.code != E34_SUCCESS) return resp_sts;

    this->writeProgramCommand(READ_CONFIGURATION);

    configuration.HEAD = saveType;

    resp_sts.code = this->sendStruct((uint8_t *)&configuration, sizeof(Configuration));
    if (resp_sts.code != E34_SUCCESS) {
        this->setMode(prevMode);
        return resp_sts;
    }

    #ifdef EBYTE_DEBUG
    DEBUG_PRINTLN(F("[E34] Set configuration"));
    this->printHead(configuration.HEAD);
    #endif

    resp_sts.code = this->setMode(prevMode);
    if (resp_sts.code != E34_SUCCESS) return resp_sts;

    if ((0xC0 != configuration.HEAD) && (0xC2 != configuration.HEAD)) {
        resp_sts.code = ERR_E34_HEAD_NOT_RECOGNIZED;
    }

    return resp_sts;
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
    DEBUG_PRINTLN(F("[E34] Module information"));
    this->printHead(moduleInformation->HEAD);
    DEBUG_PRINT(F(" Freq. : "));    DEBUG_PRINTLN(moduleInformation->frequency, HEX);
    DEBUG_PRINT(F(" Ver.  : "));    DEBUG_PRINTLN(moduleInformation->version, HEX);
    DEBUG_PRINT(F(" Features : ")); DEBUG_PRINTLN(moduleInformation->features, HEX);
    #endif

    rc.data = moduleInformation;  // malloc(sizeof (moduleInformation));
    return rc;
}

ResponseStatus Ebyte_E34::resetModule() {
    ResponseStatus resp_sts;

    resp_sts.code = checkUARTConfiguration(MODE_3_PROGRAM);
    if (resp_sts.code != E34_SUCCESS) return resp_sts;

    MODE_TYPE prevMode = this->mode;

    resp_sts.code = this->setMode(MODE_3_PROGRAM);
    if (resp_sts.code != E34_SUCCESS) return resp_sts;

    this->writeProgramCommand(WRITE_RESET_MODULE);

    resp_sts.code = this->waitCompleteResponse();
    if (resp_sts.code != E34_SUCCESS) {
        this->setMode(prevMode);
        return resp_sts;
    }

    resp_sts.code = this->setMode(prevMode);
    return resp_sts;
}


/**
 * @brief Check UART configuration for mode
 *
 * @param mode
 * @return RESPONSE_STATUS
 */
RESPONSE_STATUS Ebyte_E34::checkUARTConfiguration(MODE_TYPE mode) {
    if (mode == MODE_3_PROGRAM && this->bpsRate != EBYTE_CONFIG_BAUD) {
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
Status Ebyte_E34::sendStruct(const void * structureManaged, size_t size_of_st) {
    if (size_of_st > EBYTE_E34_MAX_LEN) {
        return ERR_E34_PACKET_TOO_BIG;
    }

    size_t len = this->hs->write((uint8_t *)structureManaged, size_of_st);
    DEBUG_PRINTF("[E34] Send struct len:%d size:%d" ENDL, len, size_of_st);

    if (len != size_of_st) {
        return (len == 0)? ERR_E34_NO_RESPONSE_FROM_DEVICE : ERR_E34_DATA_SIZE_NOT_MATCH;
    }
    return this->waitCompleteResponse();
}

Status Ebyte_E34::receiveStruct(void * structureManaged, size_t size_of_st) {
    size_t len = this->hs->readBytes((uint8_t *)structureManaged, size_of_st);
    DEBUG_PRINTF("[E34] Recv struct len:%d size:%d" ENDL, len, size_of_st);

    if (len != size_of_st) {
        return (len == 0)? ERR_E34_NO_RESPONSE_FROM_DEVICE : ERR_E34_DATA_SIZE_NOT_MATCH;
    }
    return this->waitCompleteResponse();
}


/**
 * @brief Receiving
 *
 */

ResponseContainer Ebyte_E34::receiveMessage() {
    ResponseContainer rc;
    rc.status.code = E34_SUCCESS;
    rc.data        = this->hs->readString();
    // this->cleanUARTBuffer();
    return rc;
}

ResponseStructContainer Ebyte_E34::receiveMessageFixedSize(size_t size) {
    ResponseStructContainer rc;
    rc.data        = malloc(size);
    rc.status.code = this->receiveStruct(rc.data, size);
    // this->cleanUARTBuffer();
    return rc;
}

ResponseContainer Ebyte_E34::receiveMessageUntil(char delimiter) {
    ResponseContainer rc;
    rc.status.code = E34_SUCCESS;
    rc.data        = this->hs->readStringUntil(delimiter);
    // this->cleanUARTBuffer();  <-- no flush, keep for next time
    return rc;
}

ResponseContainer Ebyte_E34::receiveMessageString(size_t size) {
    ResponseContainer rc;
    rc.status.code = E34_SUCCESS;
    char buff[size+1];
    buff[size] = '\0';  // To be sure as a null terminated string.
    size_t len = this->hs->readBytes(buff, size);
    rc.data = buff;

    if (len != size) {
        rc.status.code = (len == 0)? ERR_E34_NO_RESPONSE_FROM_DEVICE : ERR_E34_DATA_SIZE_NOT_MATCH;
    }
    return rc;
}


/**
 * @brief Sending
 */

ResponseStatus Ebyte_E34::sendMessage(const String message) {
    return this->sendMessage(message.c_str(), message.length());
}

ResponseStatus Ebyte_E34::sendMessage(const void * message, size_t size) {
    ResponseStatus resp_sts;
    resp_sts.code = this->sendStruct(message, size);
    return resp_sts;
}

ResponseStatus Ebyte_E34::sendBroadcastFixedMessage(byte CHAN, const String message) {
    return this->sendFixedMessage(EBYTE_BROADCAST_ADDR, EBYTE_BROADCAST_ADDR, CHAN, message);
}

ResponseStatus Ebyte_E34::sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const String message) {
    return this->sendFixedMessage(ADDH, ADDL, CHAN, message.c_str(), message.length());
}

ResponseStatus Ebyte_E34::sendBroadcastFixedMessage(byte CHAN, const void * message, size_t size) {
    return this->sendFixedMessage(EBYTE_BROADCAST_ADDR, EBYTE_BROADCAST_ADDR, CHAN, message, size);
}

ResponseStatus Ebyte_E34::sendFixedMessage(byte ADDH, byte ADDL, byte CHAN, const void * message, size_t size) {
    size_t message_size = sizeof(* FixedStransmission::message) * size;
    size_t packet_size = sizeof(FixedStransmission) + message_size;  // sizeof(FixedStransmission) neglect ::message !
    FixedStransmission * fixedPacket = (FixedStransmission *)malloc(packet_size);

    fixedPacket->ADDH = ADDH;
    fixedPacket->ADDL = ADDL;
    fixedPacket->CHAN = CHAN;
    memcpy(fixedPacket->message, message, message_size);

    ResponseStatus resp_sts = this->sendMessage(fixedPacket, packet_size);
    free(fixedPacket);

    return resp_sts;
}


/**
 * @brief Print debug
 *
 */

void Ebyte_E34::printHead(byte HEAD) {
    term_print(F(" HEAD : "));
    term_print(HEAD, BIN); term_print(" ");
    term_print(HEAD, DEC); term_print(" ");
    term_println(HEAD, HEX);
}

void Ebyte_E34::printParameters(struct Configuration * cfg) {
    this->printHead(cfg->HEAD);

    term_print(F(" AddH   : ")); term_println(cfg->ADDH, DEC);
    term_print(F(" AddL   : ")); term_println(cfg->ADDL, DEC);
    term_print(F(" Chan   : ")); term_println(cfg->CHAN, DEC);

    term_print(F(" Parity : ")); term_print(cfg->SPED.uartParity, BIN);   term_print(" -> "); term_println(cfg->SPED.parity_desc());
    term_print(F(" Baud   : ")); term_print(cfg->SPED.uartBaudRate, BIN); term_print(" -> "); term_println(cfg->SPED.baudrate_desc());
    term_print(F(" AirRate: ")); term_print(cfg->SPED.airDataRate, BIN);  term_print(" -> "); term_println(cfg->SPED.airrate_desc());

    term_print(F(" OptTx  : ")); term_print(cfg->OPTION.fixedTransmission, BIN);  term_print(" -> "); term_println(cfg->OPTION.fixed_tx_desc());
    term_print(F(" OptPlup: ")); term_print(cfg->OPTION.ioDriveMode, BIN);        term_print(" -> "); term_println(cfg->OPTION.io_drv_desc());
    // term_print(F(" OptWkUp: ")); term_print(cfg->OPTION.wirelessWakeupTime, BIN); term_print(" -> "); term_println(cfg->OPTION.wl_wake_desc());
    // term_print(F(" OptFEC : ")); term_print(cfg->OPTION.fec, BIN);                term_print(" -> "); term_println(cfg->OPTION.fec_desc());
    term_print(F(" OptPow : ")); term_print(cfg->OPTION.transmissionPower, BIN);  term_print(" -> "); term_println(cfg->OPTION.txpower_desc());

    term_println();
}
