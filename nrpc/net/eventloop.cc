#include "eventloop.h"
#include "base/logging.h"
#include <sys/eventfd.h>

namespace nrpc
{

    // WakeUper
    WakeUper::WakeUper(EventLoop *elp) : Channel(-1, elp)
    {
        // 创建一个用于唤醒事件循环的文件描述符
        m_fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        PLOG_FATAL_IF(m_fd == -1) << "WakeUper failed to invoke ::eventfd";
        // 设置关注的事件类型为可读
        m_event = EPOLLIN;
    }

    // setup
    void WakeUper::setup()
    {
        // 更新唤醒器关注的事件类型为可读
        m_event = EPOLLIN;
        // 更新唤醒器在事件循环中的状态
        m_elp->update_channel(this);
    }

    // wakeup
    void WakeUper::wakeup()
    {
        // 向唤醒器写入数据，以唤醒事件循环
        uint64_t wakeup_data = 1;
        int n = ::write(m_fd, static_cast<void *>(&wakeup_data), sizeof(wakeup_data));
        if (n == -1)
        {
            if (errno == EAGAIN ||      // for O_NONBLOCK IO
                errno == EWOULDBLOCK || // same as EAGAIN
                errno == EINTR)
            { // interrupt by signal
                // 忽略一些特定的错误情况
            }
            else
            {
                PLOG_FATAL << "WakeUper failed to invoke ::write";
            }
        }
    }

    // handle_events
    void WakeUper::handle_events(int events)
    {
        if (events & EPOLLIN)
        {
            // 处理可读事件，表示有数据可读，唤醒事件循环
            uint64_t wakeup_data;
            int n = ::read(m_fd, static_cast<void *>(&wakeup_data), sizeof(wakeup_data));
            if (n == -1)
            {
                if (errno == EAGAIN ||      // for O_NONBLOCK IO
                    errno == EWOULDBLOCK || // same as EAGAIN
                    errno == EINTR)
                { // sig interrupt
                    // 忽略一些特定的错误情况
                }
                else
                {
                    PLOG_FATAL << "WakeUper failed to invoke ::read";
                }
            }
        }
    }

    // EventLoop
    EventLoop::EventLoop() : m_epfd(-1), m_wakeuper(this), m_events(InitialEventSize)
    {
        // 创建 epoll 实例
        m_epfd = ::epoll_create(1);
        PLOG_FATAL_IF(m_epfd == -1) << "EventLoop failed to invoke ::epoll_create";
        // 设置唤醒器
        m_wakeuper.setup();
        // 初始化事件循环停止状态为 false
        m_is_stop.store(false);
    }

    // ~EventLoop
    EventLoop::~EventLoop()
    {
        // 关闭 epoll 实例
        if (m_epfd != -1)
        {
            ::close(m_epfd);
            m_epfd = -1;
        }
    }

    // loop
    void EventLoop::loop()
    {
        while (!m_is_stop.load())
        {
            // 计算超时时间
            int timeout_ms = DefaultEpollTimeout;
            {
                std::unique_lock<std::mutex> lg(m_timer_mtx);
                if (!m_timers.empty())
                {
                    TimeStamp now = TimeStamp::now();
                    TimeStamp latest = m_timers.get_latest_timer();
                    timeout_ms = latest - now;
                }
            }
            if (timeout_ms < 0)
            {
                timeout_ms = 0;
            }
            // 调用 epoll_wait 等待事件
            int err = ::epoll_wait(m_epfd, &m_events[0], m_events.size(), timeout_ms);
            if (err == -1)
            {
                if (errno == EAGAIN ||      // for O_NONBLOCK IO
                    errno == EWOULDBLOCK || // same as EAGAIN
                    errno == EINTR)
                { // sig interrupt
                    continue;
                }
                else
                {
                    PLOG_FATAL << "EventLoop failed to invoke ::epoll_wait";
                }
            }
            // 处理事件
            for (int ii = 0; ii < err; ++ii)
            {
                static_cast<Channel *>(m_events[ii].data.ptr)->handle_events(m_events[ii].events);
            }
            // 处理异步函数
            std::vector<std::function<void()>> funcs;
            {
                std::unique_lock<std::mutex> lg(m_funcs_mtx);
                std::swap(m_funcs, funcs);
            }
            for (auto &func : funcs)
            {
                func();
            }
            // 执行定时器任务
            TimeStamp now = TimeStamp::now();
            std::vector<TimerManager::Timer> exe_timers;
            {
                std::unique_lock<std::mutex> lg(m_timer_mtx);
                while (!m_timers.empty())
                {
                    const TimerManager::Timer &tm = m_timers.top();
                    if (tm.timestamp() < now)
                    {
                        exe_timers.push_back(tm);
                        m_timers.pop();
                    }
                    else
                    {
                        break;
                    }
                }
            }
            for (auto &tm : exe_timers)
            {
                tm.func()();
            }
        }
    }

    // stop
    void EventLoop::stop()
    {
        // 停止事件循环
        m_is_stop.store(true);
        // 唤醒事件循环
        wakeup();
        LOG_NOTICE << "EventLoop stop";
    }

    // wakeup
    void EventLoop::wakeup()
    {
        // 唤醒事件循环
        m_wakeuper.wakeup();
    }

    // update_channel
    void EventLoop::update_channel(Channel *channel)
    {
        // 更新通道在事件循环中的状态
        if (channel->sevent() != channel->event())
        {
            if (channel->sevent() == 0)
            {
                if (channel->event() != 0)
                {
                    // 添加通道到 epoll 实例中
                    _epoll_ctl(EPOLL_CTL_ADD, channel);
                }
            }
            else
            {
                if (channel->event() == 0)
                {
                    // 从 epoll 实例中删除通道
                    _epoll_ctl(EPOLL_CTL_DEL, channel);
                }
                else
                {
                    // 修改通道在 epoll 实例中的状态
                    _epoll_ctl(EPOLL_CTL_MOD, channel);
                }
            }
        }
    }

    // _epoll_ctl
    void EventLoop::_epoll_ctl(int op, Channel *channel)
    {
        // 调用 epoll_ctl 函数操作 epoll 实例中的通道
        struct epoll_event event;
        event.data.ptr = static_cast<void *>(channel);
        event.events = channel->event();

        int err = ::epoll_ctl(m_epfd, op, channel->fd(), &event);
        PLOG_FATAL_IF(err == -1) << "EventLoop failed to invoke ::epoll_ctl";

        // 更新通道在事件循环中的状态
        channel->set_sevent(channel->event());
    }

    void EventLoop::add_func(const std::function<void()> &func)
    {
        // 添加异步函数
        std::unique_lock<std::mutex> lg(m_funcs_mtx);
        m_funcs.push_back(func);
        // 唤醒事件循环
        wakeup();
    }

    void EventLoop::add_channel(Channel *channel, const TimeStamp &ts)
    {
        // 添加通道到事件循环
        std::unique_lock<std::mutex> lg(m_channels_mtx);
        m_channels[channel] = ts;
    }

    void EventLoop::remove_channel(Channel *channel, const TimeStamp &ts)
    {
        // 从事件循环中移除通道
        std::unique_lock<std::mutex> lg(m_channels_mtx);
        auto it = m_channels.find(channel);
        if (it != m_channels.end() && it->second == ts)
        {
            m_channels.erase(it);
        }
    }

    bool EventLoop::has_channel(Channel *channel, const TimeStamp &ts)
    {
        // 判断通道是否存在于事件循环中
        std::unique_lock<std::mutex> lg(m_channels_mtx);
        auto it = m_channels.find(channel);
        if (it != m_channels.end() && it->second == ts)
        {
            return true;
        }
        return false;
    }

}; // namespace nrpc