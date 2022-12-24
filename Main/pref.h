#ifndef __PREF_H__
#define __PREF_H__


#define PREF_NAME_SPACE "FLOW_CRTL"  // Max 15 chars

typedef struct {
    enum topic_code_t {
        PREF_ALL = 0,
        PREF_VERBOSE,
        PREF_AIRRATE,
        PREF_TXPOWER,
        PREF_CHANNEL,
        PREF_TBTW_RXTX,
        PREF_MSG_TYPE,
    } code;

    String desc() {
        switch (this->code) {
            case PREF_ALL:      return F("All preferences");
            case PREF_VERBOSE:  return F("Verbose pref.");
            case PREF_AIRRATE:  return F("Airrate pref.");
            case PREF_TXPOWER:  return F("TxPower pref.");
            case PREF_TBTW_RXTX:      return F("Inter-frame space pref.");
            case PREF_MSG_TYPE: return F("Msg type pref.");
            default:            return F("Not yet implemented!");
        }
    };
} preference_topic_t;

extern void pref_setup();
extern void pref_load(preference_topic_t topic = { preference_topic_t::PREF_ALL });
extern void pref_apply(preference_topic_t topic = { preference_topic_t::PREF_ALL });
extern void pref_save(preference_topic_t topic = { preference_topic_t::PREF_ALL });


#endif  // __PREF_H__
