#include "thread_pool.h"
#include <iostream>

ThreadPool::ThreadPool(size_t numThreads) : stopping(false) {
    pthread_mutex_init(&queueMutex, nullptr);
    pthread_cond_init(&queueCond, nullptr);

    workers.resize(numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        int rc = pthread_create(&workers[i], nullptr, &ThreadPool::workerEntry, this);
        if (rc != 0) {
            std::cerr << "Failed to create worker thread, error: " << rc << std::endl;
        }
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&queueCond);
}

void ThreadPool::submit(const std::function<void()>& task) {
    pthread_mutex_lock(&queueMutex);
    taskQueue.push(task);
    pthread_cond_signal(&queueCond);
    pthread_mutex_unlock(&queueMutex);
}

void ThreadPool::shutdown() {
    pthread_mutex_lock(&queueMutex);
    if (stopping) {
        pthread_mutex_unlock(&queueMutex);
        return;
    }
    stopping = true;
    pthread_cond_broadcast(&queueCond);
    pthread_mutex_unlock(&queueMutex);

    for (auto &t : workers) {
        pthread_join(t, nullptr);
    }
}

void* ThreadPool::workerEntry(void* arg) {
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    pool->workerLoop();
    return nullptr;
}

void ThreadPool::workerLoop() {
    while (true) {
        pthread_mutex_lock(&queueMutex);

        while (taskQueue.empty() && !stopping) {
            pthread_cond_wait(&queueCond, &queueMutex);
        }

        if (stopping && taskQueue.empty()) {
            pthread_mutex_unlock(&queueMutex);
            break;
        }

        auto task = taskQueue.front();
        taskQueue.pop();
        pthread_mutex_unlock(&queueMutex);

        // Execute task outside lock
        task();
    }
}