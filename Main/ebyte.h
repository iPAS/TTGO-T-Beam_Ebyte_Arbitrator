#ifndef __EBYTE_H__
#define __EBYTE_H__


#define EBYTE_E34    0
#define EBYTE_E34D27 1
#define EBYTE_E28    2

// #define EBYTE_MODULE EBYTE_E28  // XXX: module selection is here! OR put it in the Vscode's JSON configuration file.
#define EBYTE_MODULE EBYTE_E34D27

#ifndef EBYTE_MODULE
#define EBYTE_MODULE EBYTE_E34
#endif


#if EBYTE_MODULE == EBYTE_E34
#include "ebyte_e34.h"

#elif EBYTE_MODULE == EBYTE_E34D27
#include "ebyte_e34.h"

#elif EBYTE_MODULE == EBYTE_E28
#include "ebyte_e28.h"

#else
#error Incorrect EBYTE_MODULE definition !!!
#endif


class EbyteSetter {
  protected:
    uint8_t byte_param;

  public:
    EbyteSetter() = delete;
    EbyteSetter(uint8_t param) {
        this->byte_param = param;
    };
    virtual void operator () (Configuration &) = 0;
    virtual bool validate(Configuration &) = 0;
};


extern void ebyte_setup();
extern void ebyte_process();  // Store & forward data between

extern void ebyte_set_configs(EbyteSetter & setter);
extern void ebyte_apply_configs();
extern void ebyte_set_airrate(uint8_t level);
extern void ebyte_set_txpower(uint8_t level);
extern void ebyte_set_channel(uint8_t chan);

extern EbyteE34 ebyte;
extern int ebyte_show_report_count;
extern bool ebyte_loopback_flag;
extern uint8_t ebyte_airrate_level;
extern uint8_t ebyte_txpower_level;
extern uint8_t ebyte_channel;


#endif  // __EBYTE_H__
