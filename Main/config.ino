#include "global.h"

#include <Preferences.h>

static Preferences pref;


// ----------------------------------------------------------------------------
void config_setup() {
    // config_load(R_NODE_ID);
}

// ----------------------------------------------------------------------------
void config_save(preference_topic_t reg) {
    pref.begin(PREF_NAME_SPACE, false);

    // switch (reg) {
    //     case R_NODE_ID:
    //         pref.putUShort(STR(R_NODE_ID), getAddress());
    //         break;
    // }

    pref.end();
}

// ----------------------------------------------------------------------------
void config_load(preference_topic_t reg) {
    pref.begin(PREF_NAME_SPACE, true);

    // switch (reg) {
    //     case R_NODE_ID:
    //         setAddress(pref.getUShort(STR(R_NODE_ID), getAddress()));
    //         break;
    // }

    pref.end();
}
