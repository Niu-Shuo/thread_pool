#ifndef REACTOR_THREAD_POOL_WORKERTHREAD_H
#define REACTOR_THREAD_POOL_WORKERTHREAD_H

#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>
#include "EventLoop.h"

class WorkerThread
{
public:
    WorkerThread(int index);
    ~WorkerThread();

    // 线程启动,传入线程
    void run();

    inline EventLoop *getEventLoop()
    {
        return m_eventLoop;
    }

private:
    void *running();

private:
    std::thread *m_thread;          // 保存线程地址指针
    std::thread::id m_threadID;     // 线程ID
    std::string m_name;             // 线程名
    std::mutex m_mutex;             // 线程阻塞
    std::condition_variable m_cond; // 条件变量
    EventLoop *m_eventLoop;         // 反映堆reactor模型 ，线程执行什么任务，取决于往反应堆模型添加了什么数据
};

#endif