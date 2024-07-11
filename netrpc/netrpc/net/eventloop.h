/*
 * @Description: 事件循环逻辑声明，主要 eventloop.h
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-05 18:55:46
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-15 20:55:56
 */
#ifndef NETRPC_NET_EVENTLOOP_H
#define NETRPC_NET_EVENTLOOP_H

#include <memory>
#include <set>
#include <pthread.h>
#include <functional>
#include <queue>
#include <mutex>
#include "netrpc/net/fd_event.h"
#include "netrpc/net/wakeup_fd_event.h"
#include "netrpc/net/timer.h"

namespace netrpc
{
    using WakeFdEventPtr = std::shared_ptr<WakeUpFdEvent>;

    // EventLoop 类，负责管理事件循环
    class EventLoop
    {
    public:
        // 构造函数
        EventLoop();

        // 析构函数
        ~EventLoop();

        // 事件循环的主函数
        void loop();

        // 唤醒事件循环
        void wakeup();

        // 停止事件循环
        void stop();

        // 添加事件到 epoll 中
        void addEpollEvent(FdEvent *event);

        // 从 epoll 中删除事件
        void deleteEpollEvent(FdEvent *event);

        // 判断是否在事件循环的线程中
        bool isInLoopThread();

        // 添加任务到任务队列中
        void addTask(std::function<void()> cb, bool is_wake_up = false);

        // 添加定时器事件
        void addTimerEvent(TimerEvent::TimerEventPtr event);

        // 判断事件循环是否正在运行
        bool isLooping();

    public:
        // 获取当前线程的事件循环实例
        static EventLoop *GetCurrentEventLoop();

    private:
        // 处理唤醒事件
        void dealWakeup();

        // 初始化唤醒文件描述符事件
        void initWakeUpFdEvent();

        // 初始化定时器
        void initTimer();

    private:
        pid_t m_thread_id{0};                              // 线程 ID
        int m_epoll_fd{0};                                 // epoll 文件描述符
        int m_wakeup_fd{0};                                // 唤醒文件描述符
        WakeUpFdEvent *m_wakeup_fd_event{NULL};            // 唤醒文件描述符事件
        bool m_quit{false};                                // 是否退出事件循环
        std::set<int> m_listen_fds;                        // 监听的文件描述符集合
        std::queue<std::function<void()>> m_pending_tasks; // 待处理任务队列
        std::mutex m_mutex;                                // 保护任务队列的互斥锁
        Timer *m_timer{NULL};                              // 定时器
        bool m_is_looping{false};                          // 是否正在循环
    };
};

#endif