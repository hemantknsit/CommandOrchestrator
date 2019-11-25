#include <chrono>
#include <thread>
#include <iostream>
#include <boost/lockfree/queue.hpp>

#include "Orchestrator.h"
#include "Helper.h"
#include "Process.h"
#include "Command.h"
#include "Config.h"
///using namespace CommandOrchestrator::Orchestrator;

namespace CommandOrchestrator {
using namespace std;
using namespace CommandOrchestrator;
//using namespace rapidjson;
using CommandOrchestrator::Process;

const unsigned int DEFAULT_NODES_IN_QUEUE = 128;
typedef std::chrono::time_point<chrono::steady_clock> TimePoint;
boost::lockfree::queue<const Process*> processCompletionQueue(DEFAULT_NODES_IN_QUEUE);

void OnProcessExited(Process const* process) {
    processCompletionQueue.push(process);
}

//Prefix increment operator for VectorsToParse i.e. ++val
VectorsToParse operator++(VectorsToParse& val) {
    using IntType = typename std::underlying_type<VectorsToParse>::type;
    val = static_cast<VectorsToParse>((static_cast<IntType>(val) + 1));
    if (val == VectorsToParse::NUM_VECTORS_TO_PARSE)
        val = static_cast<VectorsToParse>(0);
    return val;
}

const CommandPtr_S Orchestrator::getNextCommand(void) {
    static VectorsToParse vecToParse = VectorsToParse::Commands;  //next vector to parse 
    static unsigned int nextIndexToParse = 0;    //index into relevant vector

    if (vecToParse == VectorsToParse::None) {
        //parsed all commands
        return nullptr;
    }

    const std::vector<CommandPtr_S>& vec = (vecToParse == VectorsToParse::Commands) ? _commands : _compoundCommands;
    if (nextIndexToParse < vec.size()) {
        if (nextIndexToParse == (vec.size() - 1)) { //last element of the vector
            ++vecToParse;
            nextIndexToParse = 0;
            return vec[vec.size() - 1];
        } else {
            return vec[nextIndexToParse++];
        }
    } else if (vec.size() == 0) {
        ++vecToParse;
        nextIndexToParse = 0;
        return nullptr;
    } else { 
        cerr << "Logical error, nextIndexToParse >= size for commands";
        throw LogicalErrorException("Logical error, nextIndexToParse >= size for commands");
    }
}

CommandExecutionStatus executeNextCommand(const CommandPtr_S currentCommand, vector <shared_ptr<Process>>& processes) {
    if (currentCommand) {
        shared_ptr<Process> pProcess(make_shared<Process>(currentCommand));
        if (!pProcess->createSuspended()) {
            cerr << "process creation failed for command: " << pProcess->toString() << std::endl;
            return CommandExecutionStatus::START_ERROR;
        }
        pProcess->registerExitCallback(OnProcessExited);
        pProcess->resume();
        processes.push_back(pProcess);
        return CommandExecutionStatus::STARTED;
    }
    return CommandExecutionStatus::NO_OP;
}

void printStats(const CommandExecutionStats& stats, TimePoint& timeWhenLastStatsPrinted) {
    const double STATS_PRINT_INTERVAL = 1.00; //print stats every second
    auto now = std::chrono::steady_clock::now();
    auto elapsedSeconds = now - timeWhenLastStatsPrinted;
    if (elapsedSeconds.count() > STATS_PRINT_INTERVAL) {
        cout << "comamands completed: " << stats.numCompleted << " in progress: " << stats.numInProgress << " pending: " << stats.numPending
            << " error: " << stats.numError << std::endl;
        timeWhenLastStatsPrinted = now;
    }
}

void updateStats(CommandExecutionStats& stats, CommandExecutionStatus status) {
    switch (status) {
    case CommandExecutionStatus::NO_OP:
        break;
    case CommandExecutionStatus::STARTED:
        --stats.numPending; ++stats.numInProgress;
        break;
    case CommandExecutionStatus::START_ERROR:
        --stats.numPending; ++stats.numError;
        break;
    case CommandExecutionStatus::COMPOSITE_INTERMEDIATE_ERROR:
        --stats.numInProgress; ++stats.numError;
    case CommandExecutionStatus::COMPLETED:
        --stats.numInProgress; ++stats.numCompleted;
        break;
    }
}

// return true if any work was done, else false
bool monitorProcessCompletion(vector <shared_ptr<Process>>& processes, CommandExecutionStats& stats) {
    Process* pProcess = nullptr;
    bool anyWorkDone = false;
    if (processCompletionQueue.pop(pProcess)) {
        if (pProcess->getCommand()->isPartOfCompositeCommand()) {
            if (pProcess->getCommand()->hasNext()) {
                //Part of the composite command finished running, need to start the next part, no change in stats
                if (executeNextCommand(pProcess->getCommand()->getNext(), processes) == CommandExecutionStatus::START_ERROR)
                    updateStats(stats, CommandExecutionStatus::COMPOSITE_INTERMEDIATE_ERROR);
            } else {
                //composite command finished running
                updateStats(stats, CommandExecutionStatus::COMPLETED);
            }
        } else { //non-composite command finished running
            updateStats(stats, CommandExecutionStatus::COMPLETED);
        }
        anyWorkDone = true;
    }
    return anyWorkDone;
}

// if main thread is free, sleep until time to updateStats
void possiblySleepTillStatsRequireUpdate(const CommandExecutionStatus status, bool didSomeWork, TimePoint timeWhenLastStatsPrinted) {
    if (status == CommandExecutionStatus::NO_OP && !didSomeWork) {
        auto timeToSleepUntil = timeWhenLastStatsPrinted + 1s;
        std::this_thread::sleep_until(timeToSleepUntil);
    }
}

Orchestrator& Orchestrator::getInstance(string configFilePath) {
    Orchestrator* orchestrator = new Orchestrator(configFilePath);
    return *orchestrator;
}

bool Orchestrator::isOrchestrationComplete(void) const{
    if ( (_stats.numCompleted + _stats.numError) < _numCommands)
        return false;
    return true;
}

Orchestrator::Orchestrator(string configFilePath)
    : _config(Config::getInstance(configFilePath)),
    _numCommands(_config.getNumCommands()),
    _commands(_config.getCommands()),
    _compoundCommands(_config.getCompoundCommands()),
    _stats{ 0, 0, _numCommands, 0 },
    _processes(vector<shared_ptr<Process>>()){
}

Orchestrator::~Orchestrator(void) {
    //_processes.clear();
}

void Orchestrator::run(void) {
    //vector<unique_ptr<Process>> processes;
    static auto timeWhenLastStatsPrinted = std::chrono::steady_clock::now();
    bool didSomeWork = monitorProcessCompletion(_processes, _stats);
    CommandExecutionStatus status = executeNextCommand(getNextCommand(), _processes);
    updateStats(_stats, status);
    printStats(_stats, timeWhenLastStatsPrinted);
    possiblySleepTillStatsRequireUpdate(status, didSomeWork, timeWhenLastStatsPrinted);
}

} // end of namespace CommandOrchestrator