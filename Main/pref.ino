#include "global.h"

#include <Preferences.h>

static Preferences pref;


// ----------------------------------------------------------------------------
void pref_setup() {
    // pref.clear(); pref_save();  // XXX: Used for re-initializing the preferences
    pref_load();  // No need for updating to the module again. It has been done in ebyte_setup().
}

// ----------------------------------------------------------------------------
void pref_load(preference_topic_t topic) {
    pref.begin(PREF_NAME_SPACE, true);

    // Load from flash
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_VERBOSE) {
        system_verbose_level = (verbose_level_t)pref.getUChar(STR(PREF_VERBOSE), system_verbose_level);
    }
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_AIRRATE) {
        ebyte_airrate_level = pref.getUChar(STR(PREF_AIRRATE), ebyte_airrate_level);
    }
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_TXPOWER) {
        ebyte_txpower_level = pref.getUChar(STR(PREF_TXPOWER), ebyte_txpower_level);
    }
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_CHANNEL) {
        ebyte_channel = pref.getUChar(STR(PREF_CHANNEL), ebyte_channel);
    }
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_IFS) {
        ebyte_ifs_ms = pref.getULong(STR(PREF_IFS_TYPE), ebyte_ifs_ms);
    }
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_MSG_TYPE) {
        ebyte_message_type = pref.getUChar(STR(PREF_MSG_TYPE), ebyte_message_type);
    }

    pref.end();
}

// ----------------------------------------------------------------------------
void pref_apply(preference_topic_t topic) {
    switch (topic.code) {
        case topic.PREF_ALL: ebyte_apply_configs(); break;
        case topic.PREF_VERBOSE: break;
        case topic.PREF_AIRRATE: ebyte_set_airrate(ebyte_airrate_level); break;
        case topic.PREF_TXPOWER: ebyte_set_txpower(ebyte_txpower_level); break;
        case topic.PREF_CHANNEL: ebyte_set_channel(ebyte_channel); break;
        case topic.PREF_IFS: break;
        case topic.PREF_MSG_TYPE: break;
        default: break;
    }
}

// ----------------------------------------------------------------------------
void pref_save(preference_topic_t topic) {
    pref.begin(PREF_NAME_SPACE, false);

    // Save to flash
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_VERBOSE) {
        pref.putUChar(STR(PREF_VERBOSE), system_verbose_level);
    }
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_AIRRATE) {
        pref.putUChar(STR(PREF_AIRRATE), ebyte_airrate_level);
    }
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_TXPOWER) {
        pref.putUChar(STR(PREF_TXPOWER), ebyte_txpower_level);
    }
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_CHANNEL) {
        pref.putUChar(STR(PREF_CHANNEL), ebyte_channel);
    }
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_IFS) {
        pref.putULong(STR(PREF_IFS), ebyte_ifs_ms);
    }
    if (topic.code == topic.PREF_ALL  ||  topic.code == topic.PREF_MSG_TYPE) {
        pref.putUChar(STR(PREF_MSG_TYPE), ebyte_message_type);
    }

    pref.end();
}
