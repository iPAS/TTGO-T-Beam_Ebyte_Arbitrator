#include "global.h"


#define CLI_CONSOLE Serial
#define CLI_BAUD    115200
#define CLI_PIN_RX  3
#define CLI_PIN_TX  1

SimpleCLI cli;
Command cmd_help;
Command cmd_reset;
Command cmd_ebyte_version_info;
Command cmd_verbose;
Command cmd_ebyte_airrate;
Command cmd_ebyte_txpower;
Command cmd_ebyte_channel;
Command cmd_ebyte_send;
Command cmd_ebyte_get_config;
Command cmd_pref_save_reset;
Command cmd_ebyte_show_report;
Command cmd_ebyte_loopback;
Command cmd_print_gps;
Command cmd_message_type;

#define DEFAULT_SEND_MESSAGE "0123456789"
#define DEFAULT_REPORT_COUNT 1
#define DEFAULT_PRINT_GPS_COUNT 1

const static char *help_description[] = {  // TODO: runtime configurable E34 or E28
    "\th|elp",
    "\tre|set           -- reset",
    "\ti|nfo            -- get module version infomation",
    "\tv|erbose [level] -- show or set info level [0=none | 1=err | 2=warn | 3=info | 4=debug]",

    "\ta|irrate [level] -- show or set airrate level"
        #if EBYTE_MODULE == EBYTE_E28
        " [0=auto | 1=1kbps | 2=5kbps | 3=10kbps | 4=50kbps | 5=100kbps | 6=1M(FLRC) | 7=2M(FSK)]",
        #else
        " [0=250kbps | 1=1Mbps | 2=2Mbps]",
        #endif

    "\tt|xpower [level] -- show or set txpower level"
        #if EBYTE_MODULE == EBYTE_E34
        " [0=20dBm | 1=14dBm | 2=8dBm | 3=2dBm]",
        #elif EBYTE_MODULE == EBYTE_E34D27
        " [0=27dBm | 1=21dBm | 2=15dBm | 3=9dBm]",
        #elif EBYTE_MODULE == EBYTE_E28
        " [0=12dBm | 1=10dBm | 2=7dBm | 3=4dBm]",
        #endif

    "\tch|annel [ch]    -- show or set channel [0-11]",
    "\ts|end [message]  -- send [def. \"" DEFAULT_SEND_MESSAGE "\"]",
    "\tc|onfig          -- get configuration from Ebyte directly",
    "\tp|ref [0]        -- save or reset preferences. 0:reset null:save",
    "\tr|eport [n]      -- show report n times. 0:dis -1:always [def. \"" STR(DEFAULT_REPORT_COUNT) "\"]",
    "\tl|oopback [1|0]  -- show or set the 'send-back' mode",
    "\tg|ps [n]         -- print GPS n times. 0:dis -1:always [def. \"" STR(DEFAULT_PRINT_GPS_COUNT) "\"]",
    "\tty|pe [n]        -- show or set message type [0=raw | 1=mavlink]",
};


// ----------------------------------------------------------------------------
void on_error_callback(cmd_error *e) {
    CommandError cmdError(e); // Create wrapper object
    term_println("[CLI] " + cmdError.toString());
    if (cmdError.hasCommand()) {
        term_printf("[CLI] Did you mean '%s' ?" ENDL, cmdError.getCommand().toString().c_str());
    }
}

// ----------------------------------------------------------------------------
void cli_setup() {
    CLI_CONSOLE.begin(CLI_BAUD, SERIAL_8N1, CLI_PIN_RX, CLI_PIN_TX);
    while (!CLI_CONSOLE) taskYIELD();  // Yield
    while (CLI_CONSOLE.available())
        CLI_CONSOLE.read();  // Clear buffer

    cli.setOnError(&on_error_callback); // Set error Callback

    cmd_help = cli.addCommand("h/elp", on_cmd_help);

    cmd_help = cli.addCommand("re/set", on_cmd_reset);

    cmd_ebyte_version_info = cli.addCommand("i/nfo", on_cmd_ebyte_version_info);

    cmd_verbose = cli.addCommand("v/erbose", on_cmd_verbose);
    cmd_verbose.addPositionalArgument("level", "");

    cmd_ebyte_airrate = cli.addCommand("a/irrate", on_cmd_ebyte_airrate);
    cmd_ebyte_airrate.addPositionalArgument("level", "");

    cmd_ebyte_txpower = cli.addCommand("t/xpower", on_cmd_ebyte_txpower);
    cmd_ebyte_txpower.addPositionalArgument("level", "");

    cmd_ebyte_channel = cli.addCommand("ch/annel", on_cmd_ebyte_channel);
    cmd_ebyte_channel.addPositionalArgument("ch", "");

    cmd_ebyte_send = cli.addCommand("s/end", on_cmd_ebyte_send);
    cmd_ebyte_send.addPositionalArgument("message", DEFAULT_SEND_MESSAGE);

    cmd_ebyte_get_config = cli.addCommand("c/onfig", on_cmd_ebyte_get_config);

    cmd_pref_save_reset = cli.addCommand("p/ref", on_cmd_pref_save_reset);
    cmd_pref_save_reset.addPositionalArgument("level", "");

    // cmd_ebyte_show_report = cli.addCommand("r/eport", on_cmd_ebyte_show_report);
    // cmd_ebyte_show_report.addPositionalArgument("count", STR(DEFAULT_REPORT_COUNT));
    // cmd_ebyte_show_report = cli.addBoundlessCommand("r/eport", on_cmd_ebyte_show_report);  // To be able to get -1
    cmd_ebyte_show_report = cli.addSingleArgumentCommand("r/eport", on_cmd_ebyte_show_report);  // To be able to get -1

    cmd_ebyte_loopback = cli.addCommand("l/oopback", on_cmd_ebyte_loopback);
    cmd_ebyte_loopback.addPositionalArgument("flag", "");

    cmd_print_gps = cli.addSingleArgumentCommand("g/ps", on_cmd_print_gps);  // To be able to get -1

    cmd_message_type = cli.addCommand("ty/pe", on_cmd_message_type);
    cmd_message_type.addPositionalArgument("type", "");
}

// ----------------------------------------------------------------------------
void cli_interpretation_process() {
    static String line = "";

    while (CLI_CONSOLE.available()) {
        if (CLI_CONSOLE.peek() == '\n'  ||  CLI_CONSOLE.peek() == '\r') {
            CLI_CONSOLE.read();  // Just ignore
            if (line.length() > 0) {
                CLI_CONSOLE.println();
                cli.parse(line);  // Parse the user input into the CLI
                line = "";
                break;
            }
        }
        else {
            char c = (char)CLI_CONSOLE.read();
            CLI_CONSOLE.print(c);
            line += c;
        }
    }
}

// ----------------------------------------------------------------------------
static void on_cmd_help(cmd *c) {
    uint8_t i;
    Command cmd(c);
    term_println("[CLI] Help:");
    for (i = 0; i < sizeof(help_description)/sizeof(help_description[0]); i++) {
        term_println(help_description[i]);
    }
}

// ----------------------------------------------------------------------------
static void on_cmd_reset(cmd *c) {
    term_println("[CLI] Reset... bye");
    ebyte.resetModule();
    ESP.restart();
}

// ----------------------------------------------------------------------------
void on_cmd_ebyte_version_info(cmd *c) {
    uint32_t old_baud = ebyte.getBpsRate();
    ebyte.setBpsRate(EBYTE_CONFIG_BAUD);

    String info;
    ResponseStructContainer resp;
    resp = ebyte.getVersionInfo(info);  // Get c.data from here
    resp.close();  // Clean c.data that was allocated in ::getConfiguration()

    term_print(F("[CLI] Ebyte version: "));
    if (resp.status.code == ResponseStatus::SUCCESS){
        term_println(info);
    }
    else {
        term_println(resp.status.desc());  // Description of code
    }

    ebyte.setBpsRate(old_baud);
}

// ----------------------------------------------------------------------------
static void on_cmd_verbose(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("level");
    String param = arg.getValue();

    long level;
    if (extract_int(param, &level) == false) {
        if (param != "") {
            term_print(F("[CLI] What? ..")); term_println(param);
        }
    }
    else {
        system_verbose_level = (verbose_level_t)level;
    }

    term_printf("[CLI] Verbose level=%d" ENDL, system_verbose_level);
}

// ----------------------------------------------------------------------------
static void on_cmd_ebyte_airrate(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("level");
    String param = arg.getValue();

    long level;
    if (extract_int(param, &level) == false) {
        if (param != "") {
            term_print(F("[CLI] What? ..")); term_println(param);
        }
    }
    else {
        // if (0 <= level && level <= 2) {
            ebyte_airrate_level = level;
            ebyte_set_airrate(level);
        // }
    }

    term_printf("[CLI] Ebyte airrate level=%d" ENDL, ebyte_airrate_level);
}

// ----------------------------------------------------------------------------
static void on_cmd_ebyte_txpower(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("level");
    String param = arg.getValue();

    long level;
    if (extract_int(param, &level) == false) {
        if (param != "") {
            term_print(F("[CLI] What? ..")); term_println(param);
        }
    }
    else {
        // if (0 <= level && level <= 3) {
            ebyte_txpower_level = level;
            ebyte_set_txpower(level);
        // }
    }

    term_printf("[CLI] Ebyte txpower level=%d" ENDL, ebyte_txpower_level);
}

// ----------------------------------------------------------------------------
static void on_cmd_ebyte_channel(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("ch");
    String param = arg.getValue();

    long ch;
    if (extract_int(param, &ch) == false) {
        if (param != "") {
            term_print(F("[CLI] What? ..")); term_println(param);
        }
    }
    else {
        // if (0 <= ch && ch <= 11) {
            ebyte_channel = ch;
            ebyte_set_channel(ebyte_channel);
        // }
    }

    term_printf("[CLI] Ebyte channel=%d" ENDL, ebyte_channel);
}

// ----------------------------------------------------------------------------
static void on_cmd_ebyte_send(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("message");
    String msg = arg.getValue();

    uint8_t len = msg.length();
    ResponseStatus status = ebyte.sendMessage(msg.c_str(), len);
    if (status.code != ResponseStatus::SUCCESS) {
        term_print("[CLI] Ebyte send error, E34:");
        term_println(status.desc());
    }
    else {
        term_printf("[CLI] Ebyte send %d bytes: \"%s\"" ENDL, len, msg.c_str());
    }
}

// ----------------------------------------------------------------------------
void on_cmd_ebyte_get_config(cmd *c) {
    Command cmd(c);

    uint32_t old_baud = ebyte.getBpsRate();
    ebyte.setBpsRate(EBYTE_CONFIG_BAUD);

    ResponseStructContainer resp;
    resp = ebyte.getConfiguration();  // Get c.data from here
    Configuration cfg = *((Configuration *)resp.data);  // This is a memory transfer, NOT by-reference.
                                                        // It's important get configuration pointer before all other operation.
    resp.close();  // Clean c.data that was allocated in ::getConfiguration()

    term_println(F("[CLI] Ebyte configuration: "));
    if (resp.status.code == ResponseStatus::SUCCESS){
        ebyte.printParameters(cfg);
    }
    else {
        term_println(resp.status.desc());  // Description of code
    }

    ebyte.setBpsRate(old_baud);
}

// ----------------------------------------------------------------------------
void on_cmd_pref_save_reset(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument(0);
    String param = arg.getValue();

    long level;
    if (extract_int(param, &level) == false) {
        if (param != "") {
            term_print(F("[CLI] What? ..")); term_println(param);
        }
        else {
            // null --> save
            preference_topic_t pref = { .code = pref.PREF_ALL };
            pref_save(pref);
            term_println(F("[CLI] Preferencs have been saved!"));
        }
    }
    else {
        if (level == 0) {
            // 0 --> reset
            // TODO:
            term_println(F("[CLI] Not yet implemented!"));
        }
    }
}

// ----------------------------------------------------------------------------
void on_cmd_ebyte_show_report(cmd * c) {
    Command cmd(c);
    // Argument arg = cmd.getArgument("count");
    Argument arg = cmd.getArgument(0);
    String param = arg.getValue();

    if (param == "") {
        param = STR(DEFAULT_REPORT_COUNT);
    }

    long count;
    if (extract_int(param, &count)) {
        term_printf("[CLI] Ebyte report count=%d" ENDL, count);
        ebyte_show_report_count = count;
    }
    else {
        term_print(F("[CLI] What? ..")); term_println(param);
    }
}

// ----------------------------------------------------------------------------
void on_cmd_ebyte_loopback(cmd * c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("flag");
    String param = arg.getValue();

    long flag;
    if (extract_int(param, &flag) == false) {
        if (param != "") {
            term_print(F("[CLI] What? ..")); term_println(param);
        }
    }
    else {
        ebyte_loopback_flag = (flag == 0)? false : true;
    }

    term_printf("[CLI] Ebyte loopback: %s" ENDL, (ebyte_loopback_flag)? "true" : "false");
}

// ----------------------------------------------------------------------------
void on_cmd_print_gps(cmd * c) {
    Command cmd(c);
    Argument arg = cmd.getArgument(0);
    String param = arg.getValue();

    if (param == "") {
        param = STR(DEFAULT_PRINT_GPS_COUNT);
    }

    long count;
    if (extract_int(param, &count)) {
        term_printf("[CLI] Print GPS count=%d" ENDL, count);
        gps_print_count = count;
    }
    else {
        term_print(F("[CLI] What? ..")); term_println(param);
    }
}

// ----------------------------------------------------------------------------
static void on_cmd_message_type(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("type");
    String param = arg.getValue();

    long type;
    if (extract_int(param, &type) == false) {
        if (param != "") {
            term_print(F("[CLI] What? ..")); term_println(param);
        }
    }
    else {
        ebyte_message_type = type;
    }

    term_printf("[CLI] Message type=%d" ENDL, ebyte_message_type);
}
