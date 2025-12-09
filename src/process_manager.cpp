#include "process_manager.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

ProcessManager::ProcessManager() {}

// Create a new process
pid_t ProcessManager::createProcess(const std::vector<std::string>& args) {
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Fork failed!" << std::endl;
        return -1;
    }

    if (pid == 0) {
        // Child process
        
        // Convert args to char**
        std::vector<char*> execArgs;
        for (const std::string &arg : args)
            execArgs.push_back((char*)arg.c_str());
        execArgs.push_back(nullptr);

        execvp(execArgs[0], execArgs.data());

        // Exec fails:
        perror("execvp failed");
        exit(1);
    }

    // Parent process
    ProcessInfo info;
    info.pid = pid;
    info.command = args[0];
    info.state = ProcessState::RUNNING;

    processes.push_back(info);

    return pid;
}


// Terminate process

bool ProcessManager::terminateProcess(pid_t pid) {
    if (kill(pid, SIGTERM) == 0)
        return true;

    perror("kill failed");
    return false;
}


// Update process states

void ProcessManager::updateProcessStates() {
    for (auto &proc : processes) {
        int status;
        pid_t result = waitpid(proc.pid, &status, WNOHANG);

        if (result == 0) {
            proc.state = ProcessState::RUNNING;
        }
        else if (result == proc.pid) {
            proc.state = ProcessState::TERMINATED;
        }
    }
}

// Print Process Table

void ProcessManager::printProcessTable() const {
    std::cout << "\n=== PROCESS TABLE ===\n";
    for (const auto &proc : processes) {
        std::string state;

        switch (proc.state) {
            case ProcessState::RUNNING: state = "RUNNING"; break;
            case ProcessState::STOPPED: state = "STOPPED"; break;
            case ProcessState::TERMINATED: state = "TERMINATED"; break;
        }

        std::cout << "PID: " << proc.pid
                  << " | CMD: " << proc.command
                  << " | STATE: " << state << "\n";
    }
    std::cout << "======================\n";
}