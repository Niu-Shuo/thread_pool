#ifndef REACTOR_THREAD_POOL_THREADPOOL_H
#define REACTOR_THREAD_POOL_THREADPOOL_H

#include "EventLoop.h"
#include <vector>
#include "WokerThread.h"

// 定义线程池 运行线程池，public函数取出线程池中某个子线程的反应堆实例EventLoop
class ThreadPool
{
public:
    ThreadPool(EventLoop *mainLoop, int count);
    ~ThreadPool();

    // 线程池运行
    void run();

    // 取出线程池中的某个子线层的反应堆实例
    EventLoop *takeWorkerEventLoop();

private:
    // 主线程的反应堆模型
    EventLoop *m_mainLoop;                       // 主要用作备份，负责和客户端建立连接这一件事情
    bool m_isStart;                              // 判断线程池是否启动啦
    int m_threadNum;                             // 子线程数量
    std::vector<WorkerThread *> m_workerThreads; // 工作的线程数组，工作中动态的创建，根据threadNum动态分配空间
    int m_index;                                 // 当前的线程
};

#endif