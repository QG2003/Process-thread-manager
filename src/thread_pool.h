#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <queue>
#include <functional>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    // Submit a task 
    void submit(const std::function<void()>& task);

    //finish pending tasks then exit
    void shutdown();

private:
    static void* workerEntry(void* arg);
    void workerLoop();

    std::vector<pthread_t> workers;
    std::queue<std::function<void()>> taskQueue;

    pthread_mutex_t queueMutex;
    pthread_cond_t  queueCond;

    bool stopping;
};

#endif