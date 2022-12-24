#ifndef __EBYTE_H__
#define __EBYTE_H__


#define EBYTE_E34    0
#define EBYTE_E34D27 1
#define EBYTE_E28    2

// #define EBYTE_MODULE EBYTE_E28  // XXX: module selection is here! OR put it in the Vscode's JSON configuration file.
// #define EBYTE_MODULE EBYTE_E34D27

#ifndef EBYTE_MODULE
#define EBYTE_MODULE EBYTE_E34
#endif


#if EBYTE_MODULE == EBYTE_E34
#include "ebyte_e34.h"
#define EB E34
extern EbyteE34 ebyte;

#elif EBYTE_MODULE == EBYTE_E34D27
#include "ebyte_e34.h"
#define EB E34
extern EbyteE34 ebyte;

#elif EBYTE_MODULE == EBYTE_E28
#include "ebyte_e28.h"
#define EB E28
extern EbyteE28 ebyte;

#else
#error Incorrect EBYTE_MODULE definition !!!
#endif


class EbyteSetter {

  public:
    EbyteSetter() = delete;
    EbyteSetter(uint8_t param) {
        this->byte_param = param;
    };
    virtual void operator () (Configuration &) = 0;
    virtual bool validate(Configuration &) = 0;

  protected:
    uint8_t byte_param;
};


typedef struct {
    uint32_t report_millis;
    uint32_t downlink_byte_sum;
    uint32_t uplink_byte_sum;
    uint32_t prev_arival_millis;         // Previous time the packet came
    uint32_t prev_departure_millis;         // Previous time the packet went
    uint32_t inter_arival_sum_millis;    // Cummulative sum of inter-packet arival time
    uint32_t inter_arival_count;

    uint32_t loopback_tmo_millis;  // Loopback cut-frame timeout
} ebyte_stat_t;


extern void ebyte_setup();
extern void ebyte_process();  // Store & forward data between

extern void ebyte_set_configs(EbyteSetter & setter);
extern void ebyte_apply_configs();
extern void ebyte_set_airrate(uint8_t level);
extern void ebyte_set_txpower(uint8_t level);
extern void ebyte_set_channel(uint8_t chan);

extern int ebyte_show_report_count;
extern bool ebyte_loopback_flag;
extern uint8_t ebyte_airrate_level;
extern uint8_t ebyte_txpower_level;
extern uint8_t ebyte_channel;

enum {
    MSG_TYPE_RAW = 0,
    MSG_TYPE_MAVLINK = 1,
};
extern uint8_t ebyte_message_type;
extern uint32_t ebyte_tbtw_rxtx_ms;


#endif  // __EBYTE_H__
