#include "Process.h"
using namespace std;

namespace CommandOrchestrator {

Process::Process(const CommandPtr_S& pCommand)
    :_hProcess(NULL),
    _hThread(NULL),
    _hWait(NULL),
    _exitCallback(NULL),
    _id(0), 
    _command(pCommand) {
}

Process::~Process() {
    if (_hThread) CloseHandle(_hThread);

    if (_hWait) {
        if (::UnregisterWaitEx(_hWait, INVALID_HANDLE_VALUE) == 0) // INVALID_HANDLE_VALUE means "Wait for pending callbacks"
            cerr << "UnregisterWaitEx failed with error: " << GetLastError(); 
        _hWait = NULL;
    }

    if (_hProcess) {
        DWORD code;
        if (GetExitCodeProcess(_hProcess, &code)) {
            if (code == STILL_ACTIVE) TerminateProcess(_hProcess, 1);
        }

        CloseHandle(_hProcess);
    }
    _command = nullptr;
}

DWORD Process::getId() const { 
    return _id; 
}

HANDLE Process::getHandle() const {
    return _hProcess; 
}

bool Process::createSuspended() {
    string tempc(_command->getCommandLine());
    vector<char> cmdLine(tempc.c_str(), tempc.c_str() + tempc.size());
    cmdLine.push_back('\0');
    string tempWorkingDir(_command->getCurrentWorkingDir()); 
    const char* workingDir = tempWorkingDir.empty() ? NULL : tempWorkingDir.c_str();

    unsigned int creationFlags = 
        DETACHED_PROCESS            // Do not inherit parent process's console
        | CREATE_SUSPENDED          // create in a suspended state
        /*| CREATE_SEPARATE_WOW_VDM*/;  // For 16 bit apps, launch application in a seperate Virtual dos machine thus sandboxing issues

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;
    BOOL bProcessCreated = CreateProcessA(
        NULL,                                   // executable name intentinally set NULL without which 16 bits apps might not be run
        cmdLine.data(),                         // command line including executable name. Executable name limited to MAX_PATH chars
        NULL,                                   // process security attributes
        NULL,                                   // thread security attriutes
        FALSE,                                  // inherit parent process handles
        creationFlags,                          // process creation flags
        NULL,                                   // environment block(name-value strings list)
        workingDir,                             // working directory
        &si,                                    // startup info for setting extended attributes
        &pi);                                   // process information of the started process

    if (bProcessCreated) {
        _hProcess = pi.hProcess;
        _hThread = pi.hThread;
        _id = pi.dwProcessId;
    }
    else {
        DWORD error = GetLastError();
        cerr << "CreateProcess() failed with error: " << error << " extended error: " << GetLastError() << " for command line '" 
            << cmdLine.data() << "'" << endl;
    }

    return bProcessCreated ? true : false;
}

void Process::resume() {
    ResumeThread(_hThread);
}

bool Process::registerExitCallback(ProcessExitCallback callback) {
    if (!callback) return false;
    if (_exitCallback) return false; // already registered

    _exitCallback = callback;

    BOOL result = RegisterWaitForSingleObject(&_hWait, // the new wait handle created
        _hProcess,
        OnExited,           //callback function to call when process exits
        this,               //context to pass to callback
        INFINITE,           // do not timeout, invoke the callback only when process exits
        WT_EXECUTEONLYONCE  //once the callback is called, we are done, don't wait on the handle anymore
    );
    if (!result) {
        DWORD error = GetLastError();
        cerr << "RegisterWaitForSingleObject() failed with error code " << error << endl;
    }

    return result ? true : false;
}

void CALLBACK Process::OnExited(void* context, BOOLEAN isTimeOut) {
    ((Process*)context)->OnExited();
}

void Process::OnExited() {
    _exitCallback(this);
}

string Process::toString(void) const {
    return "id: " + to_string(_id) + _command->toString();
}

CommandPtr_S Process::getCommand(){
    return _command;
}

} // endof namespace CommandOrchestrator