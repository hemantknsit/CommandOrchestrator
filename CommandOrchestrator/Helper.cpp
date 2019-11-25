#include "Command.h"

namespace CommandOrchestrator {
CommandPtr_S getCommand_test(void) {
    static unsigned int index = 0;

    switch (++index) {
    case 1: {
        CommandPtr_S pDir = make_shared<Command>("dir", "", " ", "C:\\ ", true, nullptr);
        return make_shared<Command>("cd", ".", "C:\\temp", "C:\\ ", true, pDir);
    }
    case 2:
        return make_shared<Command>("a.bat", ".", "C:\\temp", "C:\\");
    case 3:
        return make_shared<Command>("notepad.exe", ".", "C:\\temp", "C:\\");
    default: return nullptr;
    }
}

unsigned int getTotalNumCommands_test(void) {
    return 3;
}
}