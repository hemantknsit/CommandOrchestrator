#include <iostream>
#include <fstream>
#include <sstream>
#include "rapidjson/document.h"

#include "Config.h"
#include "Helper.h"

namespace CommandOrchestrator {
using namespace rapidjson;
typedef rapidjson::GenericObject<false, rapidjson::Value::ValueType> RapidJsonObjectType;

static const char* orchestratorTag("commandOrchestrator");
static const char* numCommandsTag("numCommands");
static const char* commandsTag("commands");
static const char* compoundCommandsTag("compoundCommands");
static const char* compoundCommandTag("compoundCommand");
static const char* nameTag("name");
static const char* pathTag("path");
static const char* argsTag("args");
static const char* workingDirTag("workingDir");
   
//parse an array of Commands from a parent object
template <typename T>
bool parseConfigForCommands(vector<CommandPtr_S>& commands, T& obj, const char* commandTag, bool isPartOfCompositeCommand = false) {
    if (!obj.HasMember(commandTag))
        return false; 
    
    auto cmdArray = obj[commandTag].GetArray();
    for (Value::ConstValueIterator itr = cmdArray.Begin(); itr != cmdArray.End(); ++itr) {
        if (itr->GetType() != rapidjson::kObjectType) {
            cerr << "Bad json, command is not of object type";
            return false;
        }
        auto cmdJson = itr->GetObject();
        if (!cmdJson.HasMember(nameTag) || !cmdJson.HasMember(pathTag) || !cmdJson.HasMember(argsTag) || !cmdJson.HasMember(workingDirTag)) {
            cerr << "Bad Json, command does not have all the required fields: name, path, args and workingDir";
            return false;
        }
        string nameValue = cmdJson[nameTag].GetString();
        string pathValue = cmdJson[pathTag].GetString();
        string argsValue = cmdJson[argsTag].GetString();
        string workingDirValue = cmdJson[workingDirTag].GetString();
        commands.push_back(make_shared<Command>(nameValue, pathValue, argsValue, workingDirValue, isPartOfCompositeCommand));
    }
    
    return true;
}

static bool parseConfigForCompoundCommands(std::vector<CommandPtr_S>& compoundCommands, RapidJsonObjectType& obj) {
    if (!obj.HasMember(compoundCommandsTag))
        return false; 
    auto compoundCmdArray = obj[compoundCommandsTag].GetArray();
    for (Value::ConstValueIterator itr = compoundCmdArray.Begin(); itr != compoundCmdArray.End(); ++itr) {
        if (itr->GetType() != rapidjson::kObjectType) {
            cerr << "Bad json, compoundCommands members are not of object type";
            return false;
        }
        auto compoundCmdJson = itr->GetObject();
        vector<CommandPtr_S> subcommands;
        parseConfigForCommands(subcommands, compoundCmdJson, compoundCommandTag, true);
        /* chain the commands in order of dependency*/
        for (unsigned int i = 0; i < subcommands.size(); i++) {
            if (i != subcommands.size() - 1)
                subcommands[i]->setNext(subcommands[i + 1]); // this command will be followed by the next command
        }
        compoundCommands.push_back(subcommands[0]); // the first command in the chain of commands is pushed back
    }
    return true;
}

Config::Config(string configFilePath)
    :_configFilePath(configFilePath) {
    std::ifstream t(_configFilePath);
    std::stringstream configStr;
    configStr << t.rdbuf();

    Document doc;
    doc.Parse(configStr.str().c_str());
    if (!doc.HasMember(orchestratorTag)) {
        cerr << orchestratorTag << " missing" << endl;
        throw BadConfigException(string() + orchestratorTag + " missing");
    }

    auto obj = doc[orchestratorTag].GetObject();
    if (obj.HasMember(numCommandsTag)) {
        _numCommands = obj[numCommandsTag].GetUint();
    } else {
        cerr << "Bad Json, " << numCommandsTag << " missing" << endl;
        throw BadConfigException(string() + numCommandsTag + " missing");
    }

    bool commandsReturnCode = parseConfigForCommands(_commands, obj, commandsTag);
    bool compoundCommandsReturnCode = parseConfigForCompoundCommands(_compoundCommands, obj);

    if ((!commandsReturnCode && !compoundCommandsReturnCode) || (_numCommands != _commands.size() + _compoundCommands.size())) {
        cerr << "Bad Json, either both commands & compoundCommands not found or the sum of their size did not match to numCommands" ;
        throw BadConfigException("Bad Json, either both commands & compoundCommands not found or the sum of their size did not match to numCommands");
    }
}

Config& Config::getInstance(string configFilePath) {
    static Config* config = new Config(configFilePath);
    return *config;
}

unsigned int Config::getNumCommands(void) const {
    return _numCommands;
}

const std::vector<CommandPtr_S>& Config::getCommands(void) const {
    return _commands;
}

const std::vector<CommandPtr_S>& Config::getCompoundCommands(void) const {
    return _compoundCommands;
}
}