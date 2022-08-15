#include "global.h"


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
    Serial.begin(115200);
    while (!Serial)
        vTaskDelay(1);  // Yield

    cli.setOnError(&on_error_callback); // Set error Callback

    cmd_help = cli.addCommand("help", on_cmd_help);
}

// ----------------------------------------------------------------------------
void cli_interpretation_process() {
    static String line = "";

    while (Serial.available()) {
        if (Serial.peek() == '\n'  ||  Serial.peek() == '\r') {
            Serial.read();  // Just ignore
            if (line.length() > 0) {
                cli.parse(line);  // Parse the user input into the CLI
                line = "";
                break;
            }
        }
        else {
            line += (char)Serial.read();
        }
    }
}
