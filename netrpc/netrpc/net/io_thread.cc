/*
 * @Description: 
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-07 16:51:44
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-07 17:19:16
 */
#include <assert.h>
#include "netrpc/net/io_thread.h"
#include "netrpc/common/log.h"
#include "netrpc/common/util.h"

namespace netrpc {

    // IOThread构造函数，初始化信号量并创建线程
    IOThread::IOThread() {
        int rt = sem_init(&m_init_semaphore, 0, 0); // 初始化信号量，用于线程同步
        assert(rt == 0);
        rt = sem_init(&m_start_semaphore, 0, 0); // 初始化信号量，用于线程同步
        assert(rt == 0);
        pthread_create(&m_thread, nullptr, &IOThread::Main, this); // 创建线程
        sem_wait(&m_init_semaphore); // 等待线程初始化完成
        DEBUGLOG("IOThread [%d] create success", m_thread_id);
    }

    // IOThread析构函数，释放资源
    IOThread::~IOThread() {
        m_eventloop->stop(); // 停止事件循环
        sem_destroy(&m_init_semaphore); // 销毁信号量
        sem_destroy(&m_start_semaphore); // 销毁信号量

        // 等待当前 IO 线程结束 
        pthread_join(m_thread, nullptr);

        if (m_eventloop) {
            delete m_eventloop; // 删除事件循环对象
            m_eventloop = nullptr;
        }
    }

    // 获取事件循环对象
    EventLoop *IOThread::getEventLoop() {
        return m_eventloop;
    }

    // 启动线程
    void IOThread::start() {
        DEBUGLOG("Now invoke IOThread %d", m_thread_id);
        sem_post(&m_start_semaphore); // 发送启动信号
    }

    // 等待线程结束
    void IOThread::join() {
        pthread_join(m_thread, nullptr);
    }

    // 线程主函数
    void *IOThread::Main(void *arg) {
        IOThread* thread = static_cast<IOThread*>(arg);
        thread->m_eventloop = new EventLoop();
        thread->m_thread_id = getThreadId();

        // 唤醒等待的线程
        sem_post(&thread->m_init_semaphore);

        // 让 IO 线程等待，直到我们主动启动
        DEBUGLOG("IOThread %d created, wait start semaphore", thread->m_thread_id);

        sem_wait(&thread->m_start_semaphore); // 等待启动信号

        DEBUGLOG("IOThread %d start loop", thread->m_thread_id);
        thread->m_eventloop->loop(); // 启动事件循环
        
        DEBUGLOG("IOThread %d end loop", thread->m_thread_id);
        return nullptr;
    }
};