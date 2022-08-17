#include "global.h"


#define CLI_CONSOLE Serial
#define CLI_BAUD    115200
#define CLI_PIN_RX  3
#define CLI_PIN_TX  1

SimpleCLI cli;
Command cmd_help;
Command cmd_ebyte_send;

const static char *help_description[] = {
    "\thelp",
    "\tsend [message] -- send  [default \"hello\"]",
};

// ----------------------------------------------------------------------------
void on_error_callback(cmd_error *e) {
    CommandError cmdError(e); // Create wrapper object
    term_println("[CLI] " + cmdError.toString());
    if (cmdError.hasCommand()) {
        term_printf("[CLI] Did you mean '%s' ?", cmdError.getCommand().toString().c_str());
    }
}

// ----------------------------------------------------------------------------
void cli_setup() {
    CLI_CONSOLE.begin(CLI_BAUD, SERIAL_8N1, CLI_PIN_RX, CLI_PIN_TX);
    while (!CLI_CONSOLE) taskYIELD();  // Yield
    while (CLI_CONSOLE.available())
        CLI_CONSOLE.read();  // Clear buffer

    cli.setOnError(&on_error_callback); // Set error Callback

    cmd_help = cli.addCommand("help", on_cmd_help);
    cmd_ebyte_send = cli.addCommand("send", on_cmd_ebyte_send);
    cmd_ebyte_send.addPositionalArgument("message", "hello");
}

// ----------------------------------------------------------------------------
void cli_interpretation_process() {
    static String line = "";

    while (CLI_CONSOLE.available()) {
        if (CLI_CONSOLE.peek() == '\n'  ||  CLI_CONSOLE.peek() == '\r') {
            CLI_CONSOLE.read();  // Just ignore
            if (line.length() > 0) {
                cli.parse(line);  // Parse the user input into the CLI
                line = "";
                break;
            }
        }
        else {
            line += (char)CLI_CONSOLE.read();
        }
    }
}

// ----------------------------------------------------------------------------
void on_cmd_help(cmd *c) {
    uint8_t i;
    Command cmd(c);
    term_println("[CLI] help:");
    for (i = 0; i < sizeof(help_description)/sizeof(help_description[0]); i++) {
        term_println(help_description[i]);
    }
}

// ----------------------------------------------------------------------------
void on_cmd_ebyte_send(cmd *c) {
    Command cmd(c);
    Argument arg = cmd.getArgument("message");
    String msg = arg.getValue();
    term_printf("[CLI] Ebyte send: \"%s\"\n", msg.c_str());

    uint8_t len = msg.length();
    ResponseStatus status = ebyte.sendMessage(msg.c_str(), len);
    if (status.code != E34_SUCCESS) {
        term_print("[CLI] Ebyte send error, E34:");
        term_println(status.desc());
    }
    else {
        term_printf("[CLI] Ebyte send: %d bytes\n", len);
    }
}
