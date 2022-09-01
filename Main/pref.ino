#include "global.h"

#include <Preferences.h>

static Preferences pref;


// ----------------------------------------------------------------------------
void pref_setup() {
    // pref_load(R_NODE_ID);
}

// ----------------------------------------------------------------------------
void pref_save(preference_topic_t reg) {
    pref.begin(PREF_NAME_SPACE, false);

    // switch (reg) {
    //     case R_NODE_ID:
    //         pref.putUShort(STR(R_NODE_ID), getAddress());
    //         break;
    // }

    pref.end();
}

// ----------------------------------------------------------------------------
void pref_load(preference_topic_t reg) {
    pref.begin(PREF_NAME_SPACE, true);

    // switch (reg) {
    //     case R_NODE_ID:
    //         setAddress(pref.getUShort(STR(R_NODE_ID), getAddress()));
    //         break;
    // }

    pref.end();
}
