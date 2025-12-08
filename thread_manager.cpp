#include "thread_manager.h"
#include <iostream>

ThreadManager::ThreadManager() {}

pthread_t ThreadManager::createThread(ThreadFunc func, void* arg, const std::string& name) {
    pthread_t tid;
    int rc = pthread_create(&tid, nullptr, func, arg);
    if (rc != 0) {
        std::cerr << "pthread_create failed, error: " << rc << std::endl;
        return 0;
    }

    ThreadInfo info;
    info.tid = tid;
    info.name = name;
    info.state = ThreadState::RUNNING;

    threads.push_back(info);
    return tid;
}

bool ThreadManager::joinThread(pthread_t tid) {
    for (auto &t : threads) {
        if (pthread_equal(tid, t.tid)) {
            int rc = pthread_join(tid, nullptr);
            if (rc != 0) {
                std::cerr << "pthread_join failed, error: " << rc << std::endl;
                return false;
            }
            t.state = ThreadState::COMPLETED;
            return true;
        }
    }
    std::cerr << "Thread not found\n";
    return false;
}

void ThreadManager::joinAll() {
    for (auto &t : threads) {
        if (t.state == ThreadState::RUNNING) {
            int rc = pthread_join(t.tid, nullptr);
            if (rc == 0) {
                t.state = ThreadState::COMPLETED;
            }
        }
    }
}

void ThreadManager::printThreadTable() const {
    std::cout << "\n=== THREAD TABLE ===\n";
    for (const auto &t : threads) {
        std::string state = (t.state == ThreadState::RUNNING) ? "RUNNING" : "COMPLETED";

        std::cout << "TID: " << (unsigned long)t.tid
                  << " | NAME: " << (t.name.empty() ? "<unnamed>" : t.name)
                  << " | STATE: " << state << "\n";
    }
    std::cout << "=====================\n";
}