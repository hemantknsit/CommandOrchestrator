#pragma once
#include <stdexcept>
namespace CommandOrchestrator {
enum class CommandExecutionStatus {
    NO_OP,                          // No operation was performed
    STARTED,                        // Command started
    START_ERROR,                    // Error at start of command
    COMPOSITE_INTERMEDIATE_ERROR,   // Error in running one of the intermediate commands of a composite command
    COMPLETED                       // Command completed
};

enum class VectorsToParse {
    Commands, CompositeCommands, None, NUM_VECTORS_TO_PARSE
};



typedef struct {
    unsigned int numCompleted;
    unsigned int numInProgress;
    unsigned int numPending;
    unsigned int numError;
} CommandExecutionStats;

class BadConfigException : public runtime_error {
public:
    BadConfigException(const string& msg)
        : runtime_error(msg) {}
};

class LogicalErrorException : public runtime_error {
public:
    LogicalErrorException(const string& msg)
        : runtime_error(msg) {}
};

}
