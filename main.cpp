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

//
// Example thread function used by ThreadManager.
// It simply prints whatever string message is passed into it.
//
void* simpleThreadFunc(void* arg) {
    const char* msg = static_cast<const char*>(arg);
    std::cout << "[ThreadManager] Thread says: " << msg << std::endl;
    return nullptr;
}

int main() {
    std::cout << "===== Process & Thread Manager Demo =====\n\n";

    // ----------------------------------------------------------
    // 1) PROCESS MANAGER DEMO
    // Demonstrates basic process creation using fork() + execvp().
    // ----------------------------------------------------------
    {
        std::cout << ">>> Demo: ProcessManager (running /bin/ls)\n";

        ProcessManager pm;

        // Create a new child process that runs "ls -l".
        pm.createProcess({"/bin/ls", "-l"});

        sleep(1);  // Give the child time to complete.

        // Check for terminated processes and update their state.
        pm.updateProcessStates();

        // Print a table showing the PID, command, and state of the child.
        pm.printProcessTable();
    }


    // ----------------------------------------------------------
    // 2) IPC — UNNAMED PIPE DEMO
    // Demonstrates parent <-> child communication using pipe().
    // ----------------------------------------------------------
    {
        std::cout << "\n>>> Demo: IPCManager - Unnamed pipe\n";

        Pipe p;

        // Create a standard unnamed pipe.
        if (!IPCManager::createPipe(p)) {
            std::cerr << "Failed to create unnamed pipe\n";
        } else {
            pid_t pid = fork();

            if (pid < 0) {
                perror("fork failed");

            } else if (pid == 0) {
                // ----------------------------
                // CHILD PROCESS: write to pipe
                // ----------------------------
                close(p.readFd);   // Child does not read from pipe.

                std::string msg = "Hello from child via unnamed pipe!\n";
                IPCManager::writeToPipe(p, msg);

                close(p.writeFd);
                _exit(0);  // Use _exit() in child after fork.

            } else {
                // ----------------------------
                // PARENT PROCESS: read from pipe
                // ----------------------------
                close(p.writeFd);  // Parent does not write.

                // Read message sent by the child.
                std::string received = IPCManager::readFromPipe(p, 1024);
                std::cout << "[Parent] Received: " << received;

                close(p.readFd);

                // Wait for child to terminate.
                waitpid(pid, nullptr, 0);
            }
        }
    }


    // ----------------------------------------------------------
    // 3) IPC — SHARED MEMORY DEMO
    // Demonstrates two processes sharing the same memory region.
    // Parent writes into shared memory; child reads it.
    // ----------------------------------------------------------
    {
        std::cout << "\n>>> Demo: IPCManager - Shared memory\n";

        const std::string shmName = "/my_shm_example";
        const size_t shmSize = 4096;

        // Create the shared memory object.
        int fd = IPCManager::createSharedMemory(shmName, shmSize);
        if (fd == -1) {
            std::cerr << "Failed to create shared memory\n";
        } else {
            // Map shared memory into parent's address space.
            void* addr = IPCManager::mapSharedMemory(fd, shmSize);
            if (!addr) {
                std::cerr << "Failed to mmap shared memory\n";
            } else {
                // Parent writes a message into shared memory.
                char* data = static_cast<char*>(addr);
                std::string msg = "Hello from parent via shared memory!";
                std::strncpy(data, msg.c_str(), shmSize - 1);
                data[msg.size()] = '\0';

                pid_t pid = fork();

                if (pid < 0) {
                    perror("fork failed");

                } else if (pid == 0) {
                    // --------------------------------------------------
                    // CHILD PROCESS: Open the same shared memory region
                    // and read the message put there by the parent.
                    // --------------------------------------------------
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
                    // --------------------------------------------------
                    // PARENT PROCESS: Wait for child to finish,
                    // then clean up shared memory.
                    // --------------------------------------------------
                    waitpid(pid, nullptr, 0);

                    munmap(addr, shmSize);
                    close(fd);
                    IPCManager::unlinkSharedMemory(shmName);
                }
            }
        }
    }


    // ----------------------------------------------------------
    // 4) THREAD MANAGER DEMO
    // Demonstrates creating and joining pthreads using ThreadManager.
    // ----------------------------------------------------------
    {
        std::cout << "\n>>> Demo: ThreadManager\n";

        ThreadManager tm;

        const char* msg = "Hello from a managed pthread!";
        tm.createThread(&simpleThreadFunc, (void*)msg, "simple_thread");

        // Wait for all threads to finish.
        tm.joinAll();

        // Display thread status table.
        tm.printThreadTable();
    }


    // ----------------------------------------------------------
    // 5) THREAD POOL DEMO
    // Demonstrates concurrent task execution using a worker thread pool.
    // ----------------------------------------------------------
    {
        std::cout << "\n>>> Demo: ThreadPool (4 workers, 8 tasks)\n";

        ThreadPool pool(4);  // Create pool with 4 worker threads.

        // Submit 8 tasks to the pool.
        for (int i = 0; i < 8; ++i) {
            pool.submit([i]() {
                std::cout << "[ThreadPool] Task " << i << " running\n";
                usleep(100 * 1000); // Sleep 100ms to simulate work
            });
        }

        // Gracefully shut down after all tasks complete.
        pool.shutdown();
        std::cout << "ThreadPool shutdown completed.\n";
    }

    std::cout << "\n===== Demo complete =====\n";
    return 0;
}
