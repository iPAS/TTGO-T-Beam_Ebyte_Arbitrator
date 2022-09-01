#ifndef __PREF_H__
#define __PREF_H__


#define PREF_NAME_SPACE "FLOW_CRTL"  // Max 15 chars

typedef struct {
    enum topic_code_t {
        PREF_1,
        PREF_2,
    } code;
    String desc() {
        return F("Not yet implemented!");
        return F(this->code);
    };
} preference_topic_t;


#endif  // __PREF_H__
