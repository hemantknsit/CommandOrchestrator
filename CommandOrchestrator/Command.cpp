#include "Command.h"

namespace CommandOrchestrator {

Command::Command(string commandName, string commandPath, string commandArgs, string commandWorkingDir, bool isPartOfCompositeCommand, CommandPtr_S next, bool requiresConsoleOrUI)
  : _commandName(commandName),
    _commandPath(commandPath),
    _commandArgs(commandArgs),
    _commandWorkingDir(commandWorkingDir),
    _isPartOfCompositeCommand(isPartOfCompositeCommand),
    _next((next)),
    _requiresConsoleOrUI(requiresConsoleOrUI) {
}

string Command::toString(void) const {
    return " commandName: " + _commandName + "  commandPath: " + _commandPath + " commandArgs: " + _commandArgs + " commandWorkingDir: "
        + _commandWorkingDir;
 }

string Command::getCommandLine(void) const {
    string temp = "\"";
    temp += (_commandPath.compare(".") == 0) ? _commandName : _commandPath + _commandName;
    temp += "\"";
    if (!_commandArgs.empty())
        temp += " " + _commandArgs;
    return temp;
}

CommandPtr_S Command::getNext(void) const{
    return _next;
}

void Command::setNext(CommandPtr_S next) {
    _next = next;
}

bool Command::hasNext(void) const {
    return _next ? true : false;
}

bool Command::isPartOfCompositeCommand(void) const {
    return _isPartOfCompositeCommand;
}

string Command::getCurrentWorkingDir(void) const {
    return _commandWorkingDir;
}

} //endof of namespace CommandOrchestrator
