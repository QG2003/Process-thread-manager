#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "process_manager.h"
#include "thread_manager.h"
#include "thread_pool.h"
#include "ipc_manager.h"

// Example thread function for ThreadManager 
void* simpleThreadFunc(void* arg) {
    const char* msg = static_cast<const char*>(arg);
    std::cout << "[ThreadManager] Thread says: " << msg << std::endl;
    return nullptr;
}

int main() {
    std::cout << "===== Process & Thread Manager Demo =====\n\n";

  
    // 1) ProcessManager demo

    {
        std::cout << ">>> Demo: ProcessManager (running /bin/ls)\n";

        ProcessManager pm;
        pm.createProcess({"/bin/ls", "-l"});

        sleep(1);                // allow child to run
        pm.updateProcessStates();
        pm.printProcessTable();
    }

  
    // 2) IPC: Unnamed Pipe (parent <-> child)

    {
        std::cout << "\n>>> Demo: IPCManager - Unnamed pipe\n";

        Pipe p;
        if (!IPCManager::createPipe(p)) {
            std::cerr << "Failed to create unnamed pipe\n";
        } else {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed");
            } else if (pid == 0) {
                // Child process: write to pipe
                close(p.readFd);
                std::string msg = "Hello from child via unnamed pipe!\n";
                IPCManager::writeToPipe(p, msg);
                close(p.writeFd);
                _exit(0);
            } else {
                // Parent process: read from pipe
                close(p.writeFd);
                std::string received = IPCManager::readFromPipe(p, 1024);
                std::cout << "[Parent] Received: " << received;
                close(p.readFd);
                waitpid(pid, nullptr, 0);
            }
        }
    }

    
    // 3) IPC: Shared Memory

    {
        std::cout << "\n>>> Demo: IPCManager - Shared memory\n";

        const std::string shmName = "/my_shm_example";
        const size_t shmSize = 4096;

        int fd = IPCManager::createSharedMemory(shmName, shmSize);
        if (fd == -1) {
            std::cerr << "Failed to create shared memory\n";
        } else {
            void* addr = IPCManager::mapSharedMemory(fd, shmSize);
            if (!addr) {
                std::cerr << "Failed to mmap shared memory\n";
            } else {
                // Parent writes message into shared memory
                char* data = static_cast<char*>(addr);
                std::string msg = "Hello from parent via shared memory!";
                std::strncpy(data, msg.c_str(), shmSize - 1);
                data[msg.size()] = '\0';

                pid_t pid = fork();
                if (pid < 0) {
                    perror("fork failed");
                } else if (pid == 0) {
                    // Child: open same shared memory and read
                    int childFd = shm_open(shmName.c_str(), O_RDONLY, 0666);
                    if (childFd == -1) {
                        perror("child shm_open failed");
                        _exit(1);
                    }
                    void* childAddr = mmap(nullptr, shmSize, PROT_READ, MAP_SHARED, childFd, 0);
                    if (childAddr == MAP_FAILED) {
                        perror("child mmap failed");
                        close(childFd);
                        _exit(1);
                    }
                    char* childData = static_cast<char*>(childAddr);
                    std::cout << "[Child] Read from shared memory: " << childData << std::endl;

                    munmap(childAddr, shmSize);
                    close(childFd);
                    _exit(0);
                } else {
                    // Parent: wait and clean up
                    waitpid(pid, nullptr, 0);
                    munmap(addr, shmSize);
                    close(fd);
                    IPCManager::unlinkSharedMemory(shmName);
                }
            }
        }
    }

    // 4) ThreadManager demo
 
    {
        std::cout << "\n>>> Demo: ThreadManager\n";

        ThreadManager tm;
        const char* msg = "Hello from a managed pthread!";
        tm.createThread(&simpleThreadFunc, (void*)msg, "simple_thread");
        tm.joinAll();
        tm.printThreadTable();
    }

    //
    // 5) ThreadPool demo
    // 
    {
        std::cout << "\n>>> Demo: ThreadPool (4 workers, 8 tasks)\n";

        ThreadPool pool(4);

        for (int i = 0; i < 8; ++i) {
            pool.submit([i]() {
                std::cout << "[ThreadPool] Task " << i << " running\n";
                usleep(100 * 1000); // 100ms
            });
        }

        pool.shutdown();
        std::cout << "ThreadPool shutdown completed.\n";
    }

    std::cout << "\n===== Demo complete =====\n";
    return 0;
}