#pragma once
#include <string_view>

namespace Kernel::Shell {
    //Run the interactive shell
    void run(const char* kcmdline);

    //Parse and execute a command
    void executeCommand(std::string_view cmd);
}
