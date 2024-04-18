#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include "task.hpp"
#include <queue>
#include <condition_variable>
#include <thread>

typedef struct {
    Task* first;
    Task** last;
} threadPool_Queue;

class ThreadPool {
public:
    ThreadPool(unsigned long maxNum);
    ~ThreadPool();
    void taskPost(Task* task);
    static void worker(void* arg);

private:
    std::queue<Task*> m_taskQueue;
    std::condition_variable m_cond;
    std::mutex m_mutexPool;
    unsigned long m_maxNum;
    unsigned long m_minNum;
    int m_busyNum;
    int m_aliveNum;
    std::vector<std::thread> m_threads;
};

#endif