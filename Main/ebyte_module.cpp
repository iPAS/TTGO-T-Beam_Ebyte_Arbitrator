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
#include "ebyte_module.h"


/**
 * @brief Constructor & Destructor
 */
EbyteModule::EbyteModule(HardwareSerial * serial, byte auxPin, uint8_t mPin_cnt, uint8_t * mPins, byte rxPin, byte txPin) {
    this->hs = serial;

    this->auxPin = auxPin;
    pinMode(this->auxPin, INPUT);

    this->mPin_cnt = mPin_cnt;
    this->mPins = mPins;

    this->rxPin = rxPin;
    this->txPin = txPin;

    q_init(&this->queueTx);
}

EbyteModule::~EbyteModule() {
    if (this->current_mode == NULL) {
        DEBUG_PRINTLN(F(EBYTE_LABEL "this->current_mode still NULL"));
    }
    else {
        delete this->current_mode;
    }

    while (q_length(&this->queueTx) > 0) {
        q_dequeue(&this->queueTx, NULL, 0);
    }
}


/**
 * @brief begin() should be called in setup()
 *
 * @return true
 * @return false
 */
bool EbyteModule::begin() {
    this->setBpsRate(this->bpsRate);

    this->current_mode = this->createMode();  // Factory method

    this->current_mode->setModeDefault();
    ResponseStatus status = setMode(this->current_mode);
    return status.code == ResponseStatus::SUCCESS;
}


/**
 * Methods to indicate availability & to clear the buffer
 */
int EbyteModule::available() {
    return this->hs->available();
}

void EbyteModule::flush() {
    this->hs->flush();
}

void EbyteModule::cleanUARTBuffer() {
    while (this->available()) {
        this->hs->read();
    }
}


/**
 * @brief Set/Get BPS
 *
 * @param new_bps
 */
void EbyteModule::setBpsRate(uint32_t new_bps) {
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

uint32_t EbyteModule::getBpsRate() {
    return this->bpsRate;
}


/**
 * @brief Check whether timeout or not
 */
bool EbyteModule::is_timeout(unsigned long t, unsigned long t_prev, unsigned long timeout) {
    return (((t >= t_prev) && ((t - t_prev) >= timeout)  // Normal count up
            ) ||
            ((t < t_prev)  && (((unsigned long)(0-1) - (t_prev-t) + 1) >= timeout)  // Overflow
            ))? true : false;
}


/**
 * @brief Delay with timeout
 *
 * @param timeout
 */
void EbyteModule::managedDelay(unsigned long timeout) {
    unsigned long t_prev = millis();  // It will be overflow about every 50 days.
    while (1) {
        unsigned long t = millis();
        if (this->is_timeout(t, t_prev, timeout)) {
            break;
        }
        taskYIELD();
    }
}


/**
 * @brief Is AUX pin ready?
 *
 * @param timeout
 * @return ResponseStatus
 */
ResponseStatus EbyteModule::auxReady(unsigned long timeout) {
    unsigned long t_prev = millis();
    ResponseStatus status = { .code = ResponseStatus::SUCCESS, };

    // If AUX pin was supplied, and look for HIGH state.
    // XXX: You can omit using AUX if no pins are available, but you will have to use delay() to let module finish
    while (digitalRead(this->auxPin) == LOW) {
        unsigned long t = millis();  // It will be overflow about every 50 days.

        if (is_timeout(t, t_prev, timeout)) {
            DEBUG_PRINTLN(F(EBYTE_LABEL "Wait AUX HIGH: timeout! AUX still LOW"));
            status.code = ResponseStatus::ERR_TIMEOUT;
            return status;
        }

        DEBUG_PRINTLN(F(EBYTE_LABEL "Wait AUX HIGH.."));
        taskYIELD();
    }

    DEBUG_PRINTLN(F(EBYTE_LABEL "AUX HIGH!"));
    return status;
}


/**
 * @brief Wait until module does not transmit
 *
 * @param timeout
 * @param waitNoAux
 * @return ResponseStatus
 */
ResponseStatus EbyteModule::waitCompleteResponse(unsigned long timeout, unsigned long waitNoAux) {
    ResponseStatus status = this->auxReady(timeout);

    if (status.code == ResponseStatus::ERR_NOT_IMPLEMENT) {
        // If you can't use aux pin, use 4K7 pullup with Arduino.
        // You may need to adjust this value if transmissions fail.
        this->managedDelay(waitNoAux);
        DEBUG_PRINTLN(F(EBYTE_LABEL "Wait response: no AUX pin -- just wait.."));
    }

    // As per data sheet, control after aux goes high is 2ms; so delay for at least that long
    this->managedDelay(EBYTE_EXTRA_WAIT);
    DEBUG_PRINTLN(F(EBYTE_LABEL "Wait response: complete!"));
    return status;
}


/**
 * @brief Set/Get MODE
 *
 * @param mode
 * @return ResponseStatus
 */
ResponseStatus EbyteModule::setMode(EbyteMode * mode) {
    ResponseStatus status;

    this->managedDelay(EBYTE_EXTRA_WAIT);   // Datasheet claims module needs some extra time after mode setting (2ms).
                                            // However, most of my projects uses 10 ms, but 40ms is safer.
    DEBUG_PRINT(F(EBYTE_LABEL "Mode: "));

    if (mode->isModeCorrect() == false) {
        status.code = ResponseStatus::ERR_INVALID_PARAM;
        return status;
    }

    for (uint8_t i = 0; i < this->mPin_cnt; i++) {
        uint8_t b = ((mode->getMode() >> i) & 0x1)? HIGH : LOW;
        digitalWrite(this->mPins[i], b);
        DEBUG_PRINT(b);
    }
    DEBUG_PRINTLN(mode->description());

    // The datasheet says after 2ms later, control is returned.
    // Let's give just a bit more time for this module to be active.
    this->managedDelay(EBYTE_EXTRA_WAIT);

    // Wait until AUX pin goes back to low.
    status = this->waitCompleteResponse();

    if (status.code == ResponseStatus::SUCCESS) {
        this->current_mode->setMode(mode->getMode());
    }

    DEBUG_PRINTLN(F(EBYTE_LABEL "Clear Rx buf after mode change"));
    this->cleanUARTBuffer();

    return status;
}

EbyteMode * EbyteModule::getMode() {
    return this->current_mode;
}


/**
 * @brief Write command
 *
 * @param cmd
 */
void EbyteModule::writeProgramCommand(EBYTE_COMMAND_T cmd) {
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
ResponseStructContainer EbyteModule::getConfiguration() {
    ResponseStructContainer rc;
    uint8_t prev_code = this->current_mode->getMode();
    this->current_mode->setModeConfig();

    rc.status = this->checkUARTConfiguration(this->current_mode);
    if (rc.status.code != ResponseStatus::SUCCESS) return rc;

    rc.status = this->setMode(this->current_mode);
    if (rc.status.code != ResponseStatus::SUCCESS) return rc;

    this->writeProgramCommand(READ_CONFIGURATION);

    rc.data   = malloc(sizeof(Configuration));
    rc.status = this->receiveStruct((uint8_t *)rc.data, sizeof(Configuration));
    if (rc.status.code != ResponseStatus::SUCCESS) {
        this->current_mode->setMode(prev_code);
        this->setMode(this->current_mode);
        return rc;
    }

    #ifdef EBYTE_DEBUG
    DEBUG_PRINTLN(F(EBYTE_LABEL "Get configuration"));
    this->printHead(((Configuration *)rc.data)->getHead());
    #endif

    this->current_mode->setMode(prev_code);
    rc.status = this->setMode(this->current_mode);
    if (rc.status.code != ResponseStatus::SUCCESS) return rc;

    if ((0xC0 != ((Configuration *)rc.data)->getHead()) &&
        (0xC2 != ((Configuration *)rc.data)->getHead())) {
        rc.status.code = ResponseStatus::ERR_HEAD_NOT_RECOGNIZED;
    }

    return rc;
}


/**
 * @brief Get module info.
 *
 */
ResponseStructContainer EbyteModule::getVersionInfo(String & info) {
    ResponseStructContainer rc;
    uint8_t prev_code = this->current_mode->getMode();
    this->current_mode->setModeConfig();

    rc.status = this->checkUARTConfiguration(this->current_mode);
    if (rc.status.code != ResponseStatus::SUCCESS) return rc;

    rc.status = this->setMode(this->current_mode);
    if (rc.status.code != ResponseStatus::SUCCESS) return rc;

    this->writeProgramCommand(READ_MODULE_VERSION);  // Send C3 C3 C3

    EbyteVersion * version = this->createVersion();
    rc.status = this->receiveStruct(version->getData(), version->getLength());
    if (rc.status.code != ResponseStatus::SUCCESS) {
        this->current_mode->setMode(prev_code);
        this->setMode(this->current_mode);
        return rc;
    }

    this->current_mode->setMode(prev_code);
    rc.status = this->setMode(this->current_mode);
    if (rc.status.code != ResponseStatus::SUCCESS) return rc;

    if (version->isValid() == false) {
        rc.status.code = ResponseStatus::ERR_HEAD_NOT_RECOGNIZED;
    }

    info = version->getInfo();

    #ifdef EBYTE_DEBUG
    DEBUG_PRINT(F(EBYTE_LABEL "Module information: "));
    DEBUG_PRINTLN(info);
    #endif

    rc.data = malloc(version->getLength());
    memcpy(rc.data, version->getData(), version->getLength());
    return rc;
}


/**
 * @brief Set configuration
 *
 * @param configuration
 * @param save_type
 * @return ResponseStatus
 */
ResponseStatus EbyteModule::setConfiguration(Configuration & config, EBYTE_COMMAND_T save_type) {
    ResponseStatus status;
    uint8_t prev_code = this->current_mode->getMode();
    this->current_mode->setModeConfig();

    status = this->checkUARTConfiguration(this->current_mode);
    if (status.code != ResponseStatus::SUCCESS) return status;

    status = this->setMode(this->current_mode);
    if (status.code != ResponseStatus::SUCCESS) return status;

    this->writeProgramCommand(READ_CONFIGURATION);

    config.setHead(save_type);
    status = this->sendStruct((uint8_t *)&config, sizeof(Configuration));
    if (status.code != ResponseStatus::SUCCESS) {
        this->current_mode->setMode(prev_code);
        this->setMode(this->current_mode);
        return status;
    }

    #ifdef EBYTE_DEBUG
    DEBUG_PRINTLN(F(EBYTE_LABEL "Set configuration"));
    this->printHead(config.getHead());
    #endif

    this->current_mode->setMode(prev_code);
    status = this->setMode(this->current_mode);
    if (status.code != ResponseStatus::SUCCESS) return status;

    if ((0xC0 != config.getHead()) &&
        (0xC2 != config.getHead())) {
        status.code = ResponseStatus::ERR_HEAD_NOT_RECOGNIZED;
    }

    return status;
}


/**
 * @brief Reset module
 *
 * @return ResponseStatus
 */
ResponseStatus EbyteModule::resetModule() {
    ResponseStatus status;
    uint8_t prev_code = this->current_mode->getMode();
    this->current_mode->setModeConfig();

    status = this->checkUARTConfiguration(this->current_mode);
    if (status.code != ResponseStatus::SUCCESS) return status;

    status = this->setMode(this->current_mode);
    if (status.code != ResponseStatus::SUCCESS) return status;

    this->writeProgramCommand(WRITE_RESET_MODULE);

    status = this->waitCompleteResponse();
    if (status.code != ResponseStatus::SUCCESS) {
        this->current_mode->setMode(prev_code);
        this->setMode(this->current_mode);
        return status;
    }

    status = this->setMode(this->current_mode);
    return status;
}


/**
 * @brief Check UART configuration for mode
 *
 * @param mode
 * @return ResponseStatus
 */
ResponseStatus EbyteModule::checkUARTConfiguration(EbyteMode * mode) {
    ResponseStatus status = { .code = ResponseStatus::SUCCESS, };
    if (mode->isModeConfig() && this->bpsRate != EBYTE_CONFIG_BAUD) {
        status.code = ResponseStatus::ERR_WRONG_UART_CONFIG;
    }
    return status;
}


/**
 * Method to send or receive a chunk of data provided in the struct.
 * They are my personal favorites.
 * You need not parse or worry about sprintf() inability to handle floats.
 *
 * Put your structure definition into a .h file and include in both the sender and reciever sketches.
 */
ResponseStatus EbyteModule::sendStruct(const void * structureManaged, size_t size_of_st) {
    ResponseStatus status;

    if (size_of_st > EBYTE_MODULE_BUFFER_SIZE) {
        status.code = ResponseStatus::ERR_PACKET_TOO_BIG;
        return status;
    }

    size_t len = this->hs->write((uint8_t *)structureManaged, size_of_st);
    DEBUG_PRINTF(EBYTE_LABEL "Send struct len:%d size:%d" ENDL, len, size_of_st);

    if (len != size_of_st) {
        status.code = (len == 0)? ResponseStatus::ERR_NO_RESPONSE_FROM_DEVICE : ResponseStatus::ERR_DATA_SIZE_NOT_MATCH;
        return status;
    }

    return this->waitCompleteResponse();
}

ResponseStatus EbyteModule::receiveStruct(void * structureManaged, size_t size_of_st) {
    ResponseStatus status;

    size_t len = this->hs->readBytes((uint8_t *)structureManaged, size_of_st);
    DEBUG_PRINTF(EBYTE_LABEL "Recv struct len:%d size:%d" ENDL, len, size_of_st);

    if (len != size_of_st) {
        status.code = (len == 0)? ResponseStatus::ERR_NO_RESPONSE_FROM_DEVICE : ResponseStatus::ERR_DATA_SIZE_NOT_MATCH;
        return status;
    }
    return this->waitCompleteResponse();
}


/**
 * @brief Receiving
 *
 */

ResponseContainer EbyteModule::receiveMessage() {
    ResponseContainer rc;
    rc.status.code = ResponseStatus::SUCCESS;
    rc.data        = this->hs->readString();
    // this->cleanUARTBuffer();
    return rc;
}

ResponseStructContainer EbyteModule::receiveMessageFixedSize(size_t size) {
    ResponseStructContainer rc;
    rc.data   = malloc(size);
    rc.status = this->receiveStruct(rc.data, size);
    // this->cleanUARTBuffer();
    return rc;
}

ResponseContainer EbyteModule::receiveMessageUntil(char delimiter) {
    ResponseContainer rc;
    rc.status.code = ResponseStatus::SUCCESS;
    rc.data        = this->hs->readStringUntil(delimiter);
    // this->cleanUARTBuffer();  <-- no flush, keep for next time
    return rc;
}

ResponseContainer EbyteModule::receiveMessageString(size_t size) {
    ResponseContainer rc;
    rc.status.code = ResponseStatus::SUCCESS;
    char buff[size+1];
    buff[size] = '\0';  // To be sure as a null terminated string.
    size_t len = this->hs->readBytes(buff, size);
    rc.data = buff;

    if (len != size) {
        rc.status.code = (len == 0)? ResponseStatus::ERR_NO_RESPONSE_FROM_DEVICE : ResponseStatus::ERR_DATA_SIZE_NOT_MATCH;
    }
    return rc;
}


/**
 * @brief Sending
 */

ResponseStatus EbyteModule::sendMessage(const String message) {
    return this->sendMessage(message.c_str(), message.length());
}

ResponseStatus EbyteModule::sendMessage(const void * message, size_t size) {
    ResponseStatus status = this->sendStruct(message, size);
    return status;
}

ResponseStatus EbyteModule::sendFixedTxModeMessage(byte chan, const String message) {
    return this->sendFixedTxModeMessage(EBYTE_BROADCAST_ADDR, EBYTE_BROADCAST_ADDR, chan, message);
}

ResponseStatus EbyteModule::sendFixedTxModeMessage(byte addh, byte addl, byte chan, const String message) {
    return this->sendFixedTxModeMessage(addh, addl, chan, message.c_str(), message.length());
}

ResponseStatus EbyteModule::sendFixedTxModeMessage(byte chan, const void * message, size_t size) {
    return this->sendFixedTxModeMessage(EBYTE_BROADCAST_ADDR, EBYTE_BROADCAST_ADDR, chan, message, size);
}

ResponseStatus EbyteModule::sendFixedTxModeMessage(byte addh, byte addl, byte chan, const void * message, size_t size) {
    size_t message_size = sizeof(* FixedTxModeFrame::message) * size;
    size_t packet_size = sizeof(FixedTxModeFrame) + message_size;  // sizeof(FixedStransmission) neglect ::message !
    FixedTxModeFrame * packet = (FixedTxModeFrame *)malloc(packet_size);

    packet->addr_msb = addh;
    packet->addr_lsb = addl;
    packet->channel = chan;
    memcpy(packet->message, message, message_size);

    ResponseStatus status = this->sendMessage(packet, packet_size);
    free(packet);

    return status;
}


/**
 * @brief Queue sending
 *
 */

size_t EbyteModule::lengthMessageQueueTx() {
    return q_length(&this->queueTx);
}

ResponseStatus EbyteModule::fragmentMessageQueueTx(const void * message, size_t size) {
    ResponseStatus status;
    status.code = ResponseStatus::SUCCESS;

    byte * p = (byte *)message;
    while (size > 0) {
        size_t len = (size < EBYTE_MODULE_BUFFER_SIZE)? size : EBYTE_MODULE_BUFFER_SIZE;
        size -= len;
        if (q_enqueue(&this->queueTx, p, len) == NULL) {
            status.code = ResponseStatus::ERR_BUF_TOO_SMALL;
            break;
        }
        p += len;
    }

    return status;
}

size_t EbyteModule::processMessageQueueTx() {
    if (this->lengthMessageQueueTx() > 0) {
        ResponseStatus status = this->auxReady(EBYTE_NO_AUX_WAIT);
        if (status.code == ResponseStatus::SUCCESS)
        {
            byte * p = (byte *)q_item(&this->queueTx, 0)->data;
            size_t len = q_item(&this->queueTx, 0)->len;
            status = this->sendMessage(p, len);
            if (status.code == ResponseStatus::SUCCESS) {
                q_dequeue(&this->queueTx, NULL, 0);  // Succeeded!
                return len;
            }
        }
        else {
            DEBUG_PRINT(F(EBYTE_LABEL "Process queueTx error on waiting AUX HIGH, "));
            DEBUG_PRINTLN(status.desc());
        }
    }

    return 0;
}


/**
 * @brief Print debug
 *
 */

void EbyteModule::printHead(byte head) {
    term_print(F(" HEAD : "));
    term_print(head, BIN); term_print(" ");
    term_print(head, DEC); term_print(" ");
    term_println(head, HEX);
}

void EbyteModule::printParameters(Configuration & config) {
    this->printHead(config.getHead());

    term_print(F(" AddH   : ")); term_println(config.addr_msb, DEC);
    term_print(F(" AddL   : ")); term_println(config.addr_lsb, DEC);
    term_print(F(" Chan   : ")); term_println(config.channel, DEC);

    term_print(F(" Parity : ")); term_print(config.SPED.uartParity, BIN);   term_print(" -> "); term_println(config.SPED.parity_desc());
    term_print(F(" Baud   : ")); term_print(config.SPED.uartBaudRate, BIN); term_print(" -> "); term_println(config.SPED.baudrate_desc());
    term_print(F(" AirRate: ")); term_print(config.SPED.airDataRate, BIN);  term_print(" -> "); term_println(config.SPED.airrate_desc());

    term_print(F(" OpTxMod: ")); term_print(config.OPTION.fixedTransmission, BIN); term_print(" -> "); term_println(config.OPTION.fixed_tx_desc());
    term_print(F(" OpPlup : ")); term_print(config.OPTION.ioDriveMode, BIN);       term_print(" -> "); term_println(config.OPTION.io_drv_desc());
    term_print(F(" OpTxPow: ")); term_print(config.OPTION.transmissionPower, BIN); term_print(" -> "); term_println(config.OPTION.txpower_desc());

    term_println();
}
