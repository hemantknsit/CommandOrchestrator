#pragma once
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <stdlib.h>
#include <time.h>
#include "Command.h"

namespace CommandOrchestrator {
class Process
{
public: // type definitions
    typedef void(*ProcessExitCallback)(Process const*);

public: // functions
    Process(const CommandPtr_S& pCommand);
    ~Process();
    DWORD getId() const; 
    HANDLE getHandle() const;
    bool createSuspended(); 
    void resume(); 
    bool registerExitCallback(ProcessExitCallback callback);
    string toString() const;
    CommandPtr_S getCommand();

private: //functions
    static void CALLBACK OnExited(void* context, BOOLEAN isTimeOut);
    void OnExited();

private: // member variables
    DWORD                   _id;
    HANDLE                  _hProcess;
    HANDLE                  _hThread;
    HANDLE                  _hWait;
    ProcessExitCallback     _exitCallback;
    CommandPtr_S            _command;
}; // class Process
}  // namespace CommandOrchestrator 
