#ifndef NRPC_NET_EVENTLOOP_H
#define NRPC_NET_EVENTLOOP_H

#include <vector>
#include <sys/epoll.h>
#include <atomic>
#include "channel.h"
#include <functional>
#include <mutex>
#include "timer_manager.h"
#include <map>

namespace nrpc
{
    class Channel;

    class WakeUper : public Channel
    {
    public:
        // 构造函数
        WakeUper(EventLoop *elp);
        ~WakeUper() {}

        // 初始化唤醒器
        void setup();

        // 唤醒事件循环
        void wakeup();

        // 处理事件
        void handle_events(int events);
    };

    // 事件循环类
    class EventLoop
    {
    public:
        // 构造函数
        EventLoop();

        // 析构函数
        ~EventLoop();

        // 启动事件循环
        void loop();

        // 停止事件循环
        void stop();

        // 唤醒事件循环
        void wakeup();

        // 更新通道状态
        void update_channel(Channel *channel);

    private:
        // epoll_ctl封装
        void _epoll_ctl(int op, Channel *channel);

    public:
        // 在其他线程中调用，线程安全，添加异步函数
        void add_func(const std::function<void()> &func);

        // 在一定时间后执行函数
        void run_after(int after_time_ms, const std::function<void()> &func)
        {
            TimeStamp ts = TimeStamp::after_now_ms(after_time_ms);
            {
                std::unique_lock<std::mutex> lg(m_timer_mtx);
                m_timers.add_timer(ts, func);
            }
            wakeup();
        }

        // 添加通道到事件循环
        void add_channel(Channel *channel, const TimeStamp &ts);

        // 从事件循环中移除通道
        void remove_channel(Channel *channel, const TimeStamp &ts);

        // 判断通道是否存在于事件循环中
        bool has_channel(Channel *channel, const TimeStamp &ts);

    public:
        static const size_t InitialEventSize = 1024; // 初始事件数量
        static const int DefaultEpollTimeout = 256;  // 默认epoll超时时间（毫秒）

    private:
        int m_epfd;                               // epoll文件描述符
        WakeUper m_wakeuper;                      // 唤醒器
        std::vector<struct epoll_event> m_events; // epoll事件数组
        std::atomic<bool> m_is_stop;              // 事件循环停止标志，线程安全

        // 异步调用更改EventLoop状态
        std::vector<std::function<void()>> m_funcs;
        std::mutex m_funcs_mtx;

        // 定时器管理
        TimerManager m_timers;
        std::mutex m_timer_mtx;

        // 通道管理
        std::map<Channel *, TimeStamp> m_channels;
        std::mutex m_channels_mtx;
    };
}; // namespace nrpc

#endif