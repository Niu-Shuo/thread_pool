#include "thread_pool.hpp"
#include <iostream>


ThreadPool::ThreadPool(unsigned long maxNum) {
    m_maxNum = maxNum;
    m_minNum = 1;
    m_busyNum = 0;
    m_aliveNum = 0;

    m_threads.resize(m_maxNum);
    for(unsigned long ii = 0; ii != m_maxNum; ++ii) {
        m_threads[ii] = std::thread(worker, this);
    }
}
ThreadPool::~ThreadPool() {
    m_cond.notify_all();    // 唤醒阻塞的线程
    for(unsigned long ii = 0; ii != m_maxNum; ++ii){
        if(m_threads[ii].joinable()) {
            m_threads[ii].join();
        }
    }
}

void ThreadPool::taskPost(Task *task) {
    std::unique_lock<std::mutex> lock(m_mutexPool);
    m_taskQueue.push(task);
    m_cond.notify_one();
    lock.unlock();
}

void ThreadPool::worker(void *arg) {
    ThreadPool* pool = static_cast<ThreadPool*> (arg);
    while(1) {
        std::unique_lock<std::mutex> lock(pool->m_mutexPool);
        while(pool->m_taskQueue.empty()) {
            pool->m_cond.wait(lock);
        }

        Task* task = pool->m_taskQueue.front();
        pool->m_taskQueue.pop();

        lock.unlock();
        // 获取当前时间点
        auto now = std::chrono::system_clock::now();
    
        // 将当前时间点转换为 time_t 类型
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    
        // 将 time_t 类型转换为本地时间的 struct tm 结构体
        struct std::tm* now_tm = std::localtime(&now_c);
        
        std::cout << "执行任务的开始时间为："
              << (now_tm->tm_year + 1900) << "-" // 年
              << (now_tm->tm_mon + 1) << "-"     // 月
              << now_tm->tm_mday << " "          // 日
              << now_tm->tm_hour << ":"          // 时
              << now_tm->tm_min << ":"           // 分
              << now_tm->tm_sec << std::endl;    // 秒
        task->hander(task->m_arg);
        std::cout << task->m_name << " finish! " << std::endl;  
        delete task;
    }
}
