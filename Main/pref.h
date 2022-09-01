#ifndef __PREF_H__
#define __PREF_H__


#define PREF_NAME_SPACE "FLOW_CRTL"  // Max 15 chars

typedef struct {
    enum topic_code_t {
        PREF_ALL = 0,
        PREF_VERBOSE,
        PREF_AIRRATE,
        PREF_TXPOWER,
    } code;

    String desc() {
        switch (this->code) {
            case PREF_ALL:      return F("Not yet implemented!");  // TODO:
            case PREF_VERBOSE:  return F("Not yet implemented!");
            case PREF_AIRRATE:  return F("Not yet implemented!");
            case PREF_TXPOWER:  return F("Not yet implemented!");
            default:            return F("Not yet implemented!");
        }
    };

} preference_topic_t;

extern void pref_setup();
extern void pref_load(bool do_save = true, preference_topic_t topic = { preference_topic_t::PREF_ALL });
extern void pref_save(preference_topic_t topic = { preference_topic_t::PREF_ALL });


#endif  // __PREF_H__
