#pragma once
#include <string>
#include <vector>

#include "Command.h"

namespace CommandOrchestrator {
class Config {
public:  //functions
    static Config& getInstance(string configFilePath);
    unsigned int getNumCommands() const;
    const std::vector<CommandPtr_S>& getCommands() const;
    const std::vector<CommandPtr_S>& getCompoundCommands() const;

private:  //functions
    Config(string configFilePath);
    
private:  //member variables
    string                              _configFilePath;
    unsigned int                        _numCommands;
    std::vector<CommandPtr_S>           _commands;
    std::vector<CommandPtr_S>           _compoundCommands;
};
}