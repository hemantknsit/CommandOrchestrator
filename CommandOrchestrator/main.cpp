#include "Process.h"
#include "Command.h"
#include "Helper.h"
#include "Orchestrator.h"

#include <chrono>
#include <iostream>

using namespace std;
using namespace CommandOrchestrator;

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: commandOrchestrator ConfigFilePath" << endl;
        return -1;
    }
    cout << "CommandOrchestrator v1.0. Stats printed every 1 second" << endl;
    try {
        Orchestrator orchestrator = Orchestrator::getInstance(argv[1]);
        while (!orchestrator.isOrchestrationComplete()) {
            orchestrator.run();
        }
    } catch (exception ex) {
        cerr << "Got exception: " << ex.what() << endl;
    }

    cout << "Orchestration done. Press enter to exit" << endl;
    cin.get(); // wait for Enter
    return 0;
}

