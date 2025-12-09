#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <pthread.h>
#include <string>
#include <vector>

enum class ThreadState {
    RUNNING,
    COMPLETED
};

struct ThreadInfo {
    pthread_t tid;
    std::string name;
    ThreadState state;
};

using ThreadFunc = void* (*)(void*);

class ThreadManager {
public:
    ThreadManager();

    // Create a new thread running `func(arg)`
    pthread_t createThread(ThreadFunc func, void* arg, const std::string& name = "");

    // Join a specific thread
    bool joinThread(pthread_t tid);

    // Join all threads
    void joinAll();

    // Print a table of all threads
    void printThreadTable() const;

private:
    std::vector<ThreadInfo> threads;
};

#endif