#pragma once
#pragma once
#include <string>
#include "Process.h"
#include "Command.h"
#include "Config.h"
#include "Helper.h"

namespace CommandOrchestrator {
class Orchestrator {
public: //functions
    static Orchestrator& getInstance(string configFilePath);
    bool isOrchestrationComplete(void) const;
    void run(void);
    ~Orchestrator(void);

private: // functions
    Orchestrator(string configFilePath);
    const CommandPtr_S getNextCommand(void);

private: // members
    Config                              _config;
    unsigned int                        _numCommands;
    const std::vector<CommandPtr_S>     _commands;
    const std::vector<CommandPtr_S>     _compoundCommands;
    vector <shared_ptr<Process>>        _processes; //should be a unique_ptr, VS 2k19 complains at end of main for deleted function with unique_ptr, check with VS2k15

    CommandExecutionStats               _stats;
};
}
