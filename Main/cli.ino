#include "global.h"


#define CLI_CONSOLE Serial
#define CLI_BAUD    115200
#define CLI_PIN_RX  3
#define CLI_PIN_TX  1

SimpleCLI cli;
Command cmd_help;
Command cmd_verbose;
Command cmd_ebyte_send;
Command cmd_ebyte_get_config;
Command cmd_ebyte_show_report;
Command cmd_ebyte_loopback;

#define DEFAULT_SEND_MESSAGE "0123456789"
#define DEFAULT_REPORT_COUNT 1
#define DEFAULT_VERBOSE_LEVEL 1

const static char *help_description[] = {
    "\thelp",
    "\tverbose [level] -- show information by level [default \"" STR(DEFAULT_VERBOSE_LEVEL) "\"]",
    "\tsend [message] -- send [default \"" DEFAULT_SEND_MESSAGE "\"]",
    "\tconfig         -- get configuration",
    "\treport [count] -- show report. 0:dis -1:always [default \"" STR(DEFAULT_REPORT_COUNT) "\"]",
    "\tloopback [1|0] -- show, ena, dis the 'send-back' mode",
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

    cmd_verbose = cli.addCommand("v/erbose", on_cmd_verbose);
    cmd_verbose.addPositionalArgument("level", STR(DEFAULT_VERBOSE_LEVEL));

    cmd_ebyte_send = cli.addCommand("s/end", on_cmd_ebyte_send);
    cmd_ebyte_send.addPositionalArgument("message", DEFAULT_SEND_MESSAGE);

    cmd_ebyte_get_config = cli.addCommand("c/onfig", on_cmd_ebyte_get_config);

    // cmd_ebyte_show_report = cli.addCommand("r/eport", on_cmd_ebyte_show_report);
    // cmd_ebyte_show_report.addPositionalArgument("count", STR(DEFAULT_REPORT_COUNT));
    // cmd_ebyte_show_report = cli.addBoundlessCommand("r/eport", on_cmd_ebyte_show_report);  // To be able to get -1
    cmd_ebyte_show_report = cli.addSingleArgumentCommand("r/eport", on_cmd_ebyte_show_report);  // To be able to get -1

    cmd_ebyte_loopback = cli.addCommand("l/oopback", on_cmd_ebyte_loopback);
    cmd_ebyte_loopback.addPositionalArgument("flag", "");
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
static void on_cmd_verbose(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("level");
    String param = arg.getValue();

    if (param == "") {
        param = STR(DEFAULT_VERBOSE_LEVEL);
    }

    long level;
    if (extract_int(param, &level)) {
        term_printf("[CLI] Verbose level=%d" ENDL, level);
        system_verbose_level = level;
    }
    else {
        term_print(F("[CLI] What? ..")); term_println(param);
    }
}

// ----------------------------------------------------------------------------
static void on_cmd_ebyte_send(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("message");
    String msg = arg.getValue();

    uint8_t len = msg.length();
    ResponseStatus status = ebyte.sendMessage(msg.c_str(), len);
    if (status.code != E34_SUCCESS) {
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
    ebyte.changeBpsRate(EBYTE_CONFIG_BAUD);

    ResponseStructContainer resp;
    resp = ebyte.getConfiguration();  // Get c.data from here
    Configuration cfg = *((Configuration *)resp.data); // This is a memory transfer, NOT by-reference.
                                                    // It's important get configuration pointer before all other operation.
    resp.close();  // Clean c.data that was allocated in ::getConfiguration()

    if (resp.status.code == E34_SUCCESS){
        term_println(F("[CLI] Ebyte configuration"));
        ebyte.printParameters(&cfg);
    }
    else {
        term_println(resp.status.desc());  // Description of code
    }

    ebyte.changeBpsRate(old_baud);
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
