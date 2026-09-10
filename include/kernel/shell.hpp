#pragma once
#include <string_view>

namespace Kernel::Shell {
    //Run the interactive shell
    void run();

    //Parse and execute a command
    void executeCommand(std::string_view cmd);
}
