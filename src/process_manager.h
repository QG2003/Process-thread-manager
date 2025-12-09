#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <string>
#include <vector>
#include <sys/types.h>

enum class ProcessState {
    RUNNING,
    STOPPED,
    TERMINATED
};

struct ProcessInfo {
    pid_t pid;
    std::string command;
    ProcessState state;
};

class ProcessManager {
public:
    ProcessManager();

    pid_t createProcess(const std::vector<std::string>& args);
    bool terminateProcess(pid_t pid);
    void updateProcessStates();
    void printProcessTable() const;

private:
    std::vector<ProcessInfo> processes;
};

#endif
