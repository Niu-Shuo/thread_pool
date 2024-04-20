#include "WokerThread.h"

WorkerThread::WorkerThread(int index)
{
    m_eventLoop = nullptr;
    m_thread = nullptr;
    m_threadID = std::thread::id();
    m_name = "SubThread-" + std::to_string(index);
}

WorkerThread::~WorkerThread()
{
    // 子线程一直在线程池中运行，不会析构
    if (m_thread != nullptr)
    {
        delete m_thread;
    }
}

void WorkerThread::run()
{
    // 创建子线程,3,4子线程的回调函数以及传入的参数
    // 调用的函数，以及此函数的所有者this
    m_thread = new std::thread(&WorkerThread::running, this);

    // 阻塞主线程，让当前函数不会直接结束，不知道当前子线程是否运行结束
    // 如果为空，子线程还没有初始化完毕,让主线程等一会，等到初始化完毕
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_eventLoop == nullptr)
    {
        m_cond.wait(lock);
    }
}

void *WorkerThread::running()
{
    m_mutex.lock();                      // 阻塞
    m_eventLoop = new EventLoop(m_name); // 对evLoop做初始化
    m_mutex.unlock();
    m_cond.notify_one(); // 唤醒一个主线程的条件变量等待解除阻塞
    m_eventLoop->run();  // 启动反应堆模型
}
