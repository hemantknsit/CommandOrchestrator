#pragma once
#include <string>
#include <memory>
using namespace std;

namespace CommandOrchestrator {

enum class CommandStatus {
    YET_TO_START, STARTED, FINISHED
};

class Command;
typedef shared_ptr<Command> CommandPtr_S;

class Command {
public: //functions
    Command(string commandName, string commandPath, string commandArgs, string commandWorkingDir, bool isPartOfCompositeCommand = false, CommandPtr_S next = nullptr, bool requiresConsoleOrUI = false);
    string      toString(void) const;
    string      getCommandLine(void) const;
    string      getCurrentWorkingDir(void) const;
    CommandPtr_S getNext() const; //get nextCommand in case of interdependent commands, nullptr otherwise
    void        setNext(CommandPtr_S next);
    bool        hasNext() const;
    bool        isPartOfCompositeCommand() const;

private: //members
    string          _commandName;
    string          _commandPath;
    string          _commandArgs;
    string          _commandWorkingDir;
    bool            _requiresConsoleOrUI;
    CommandPtr_S    _next;   //next command to call in case of interDependent commands, nullptr otherwise
    bool            _isPartOfCompositeCommand;
}; //endof class Command

} //endof namespace COmmandOrchestrator
