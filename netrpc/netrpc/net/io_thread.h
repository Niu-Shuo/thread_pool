/*
 * @Description: IO线程类的定义
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-07 16:47:05
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-07 17:14:04
 */
#ifndef NETRPC_NET_IO_THREAD_H
#define NETRPC_NET_IO_THREAD_H

#include <pthread.h>
#include <semaphore.h>
#include <thread>
#include <memory>
#include <functional>
#include "netrpc/net/eventloop.h"

namespace netrpc
{

    // IOThread类，用于管理和运行事件循环线程
    class IOThread
    {
    public:
        using ThreadFunc = std::function<void()>; // 定义线程函数类型

        // 构造函数，初始化信号量
        IOThread();

        // 析构函数，释放资源
        ~IOThread();

        // 获取事件循环对象
        EventLoop *getEventLoop();

        // 启动线程
        void start();

        // 等待线程结束
        void join();

    public:
        // 线程入口函数
        static void *Main(void *arg);

    private:
        pid_t m_thread_id{-1};           // 线程ID
        pthread_t m_thread{0};           // POSIX线程句柄
        EventLoop *m_eventloop{nullptr}; // 事件循环指针
        sem_t m_init_semaphore;          // 用于线程初始化同步的信号量
        sem_t m_start_semaphore;         // 用于线程启动同步的信号量
    };
};

#endif