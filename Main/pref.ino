#include "global.h"

#include <Preferences.h>

static Preferences pref;


// ----------------------------------------------------------------------------
void pref_setup() {
    pref_load();
}

// ----------------------------------------------------------------------------
void pref_load(preference_topic_t topic) {
    pref.begin(PREF_NAME_SPACE, true);

    // Load from flash
    if (topic.code == topic.PREF_VERBOSE  ||
        topic.code == topic.PREF_ALL) {
        system_verbose_level = (verbose_level_t)pref.getUChar(STR(PREF_VERBOSE), system_verbose_level);
    }
    if (topic.code == topic.PREF_AIRRATE  ||
        topic.code == topic.PREF_ALL) {
        ebyte_airrate_level = pref.getUChar(STR(PREF_AIRRATE), ebyte_airrate_level);
    }
    if (topic.code == topic.PREF_TXPOWER  ||
        topic.code == topic.PREF_ALL) {
        ebyte_txpower_level = pref.getUChar(STR(PREF_TXPOWER), ebyte_txpower_level);
    }

    // Apply them
    switch (topic.code) {
        case topic.PREF_ALL: ebyte_apply_configs(); break;
        case topic.PREF_VERBOSE: break;
        case topic.PREF_AIRRATE: ebyte_set_airrate(ebyte_airrate_level); break;
        case topic.PREF_TXPOWER: ebyte_set_txpower(ebyte_txpower_level); break;
        default: break;
    }

    pref.end();
}

// ----------------------------------------------------------------------------
void pref_save(preference_topic_t topic) {
    pref.begin(PREF_NAME_SPACE, false);

    // Save to flash
    if (topic.code == topic.PREF_VERBOSE  ||
        topic.code == topic.PREF_ALL) {
        pref.putUChar(STR(PREF_VERBOSE), system_verbose_level);
    }
    if (topic.code == topic.PREF_AIRRATE  ||
        topic.code == topic.PREF_ALL) {
        pref.putUChar(STR(PREF_AIRRATE), ebyte_airrate_level);
    }
    if (topic.code == topic.PREF_TXPOWER  ||
        topic.code == topic.PREF_ALL) {
        pref.putUChar(STR(PREF_TXPOWER), ebyte_txpower_level);
    }

    pref.end();
}
