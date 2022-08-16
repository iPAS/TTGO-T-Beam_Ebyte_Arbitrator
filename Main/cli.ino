#include "global.h"


#define CLI_CONSOLE Serial
#define CLI_BAUD    115200
#define CLI_PIN_RX  3
#define CLI_PIN_TX  1

SimpleCLI cli;
Command cmd_help;

// ----------------------------------------------------------------------------
void on_error_callback(cmd_error *e) {
    CommandError cmdError(e); // Create wrapper object
    term_println("[CLI] " + cmdError.toString());
    if (cmdError.hasCommand()) {
        term_printf("[CLI] Did you mean '%s' ?", cmdError.getCommand().toString().c_str());
    }
}

// ----------------------------------------------------------------------------
void on_cmd_help(cmd *c) {
    const char *desc[] = {
        "\thelp",
    };
    uint8_t i;
    Command cmd(c);
    term_println("[CLI] help:");
    for (i = 0; i < sizeof(desc)/sizeof(desc[0]); i++) {
        term_println(desc[i]);
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
