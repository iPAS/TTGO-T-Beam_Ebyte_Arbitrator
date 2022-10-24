#ifndef __EBYTE_H__
#define __EBYTE_H__


#include "ebyte_module.h"


class EbyteSetter {
  protected:
    uint8_t byte_param;

  public:
    EbyteSetter() = delete;
    EbyteSetter(uint8_t param) {
        this->byte_param = param;
    };
    virtual void operator ()(Configuration *) = 0;
    virtual bool validate(Configuration *) = 0;
};


extern void ebyte_setup();
extern void ebyte_process();  // Store & forward data between

extern void ebyte_set_configs(EbyteSetter & setter);
extern void ebyte_apply_configs();
extern void ebyte_set_airrate(uint8_t level);
extern void ebyte_set_txpower(uint8_t level);
extern void ebyte_set_channel(uint8_t ch);

extern EbyteModule ebyte;
extern int ebyte_show_report_count;
extern bool ebyte_loopback_flag;
extern uint8_t ebyte_airrate_level;
extern uint8_t ebyte_txpower_level;
extern uint8_t ebyte_channel;


#endif  // __EBYTE_H__
