/*
 * @Description: timer.cc 定时器类实现
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-05 19:50:27
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-15 20:51:58
 */
#include <sys/timerfd.h>
#include "netrpc/net/timer.h"
#include "netrpc/common/log.h"
#include "netrpc/common/util.h"

namespace netrpc {

    // Timer 类构造函数，创建定时器并设置监听
    Timer::Timer() : FdEvent() {
        // 创建一个非阻塞和执行时关闭的定时器文件描述符
        m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        DEBUGLOG("timer fd = %d", m_fd);

        // 将定时器的读事件放到 eventloop 上监听
        // 设置监听事件以及对应的回调函数
        listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer, this));
    }

    // Timer 类析构函数
    Timer::~Timer() {}

    // 添加定时事件
    // 将新的定时事件添加到定时器队列中，如果新的定时事件比当前队列中的任何事件都早到期，则需要重置定时器的超时时间。
    void Timer::addTimerEvent(TimerEvent::TimerEventPtr event) {
        bool is_reset_timerfd = false;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            // 如果定时事件队列为空，或者新的定时事件比当前最早的事件更早到期
            if (m_pending_events.empty()) {
                is_reset_timerfd = true;
            } else {
                auto it = m_pending_events.begin();
                if ((*it).second->getArriveTime() > event->getArriveTime()) {
                    is_reset_timerfd = true;
                }
            }
            // 将新的定时事件添加到队列中
            m_pending_events.emplace(event->getArriveTime(), event);
        }
        // 如果需要重置定时器的超时时间，则调用 resetArriveTime 方法
        if (is_reset_timerfd) {
            resetArriveTime();
        }
    }

    // 删除定时事件
    // 将指定的定时事件标记为取消，并从定时事件队列中移除。
    void Timer::deleteTimerEvent(TimerEvent::TimerEventPtr event) {
        // 将定时事件标记为取消
        event->setCancled(true);
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            // 获取定时事件在队列中的范围
            auto begin = m_pending_events.lower_bound(event->getArriveTime());
            auto end = m_pending_events.upper_bound(event->getArriveTime());

            // 遍历范围内的定时事件，找到并删除指定的定时事件
            auto it = begin;
            for (; it != end; ++it) {
                if (it->second == event) {
                    break;
                }
            }
            if (it != end) {
                m_pending_events.erase(it);
            }
        }
    }

    // 定时器触发时的回调函数
    // 该函数在定时器触发时被调用，读取定时器文件描述符，处理所有到期的定时事件，并重新设置下一个定时事件的触发时间。
    void Timer::onTimer() {
        DEBUGLOG("ontimer");
        char buf[8];
        // 读取定时器文件描述符，直到读取完所有数据
        while (1) {
            if ((read(m_fd, buf, 8) == -1) && errno == EAGAIN) {
                break;
            }
        }

        int64_t now = getNowMs();
        std::vector<TimerEvent::TimerEventPtr> tmps;
        std::vector<std::pair<int64_t, std::function<void()>>> tasks;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            // 遍历所有待处理的定时事件
            auto it = m_pending_events.begin();
            for (; it != m_pending_events.end(); ++it) {
                if (it->first <= now) {
                    if (!it->second->isCancled()) {
                        tmps.push_back(it->second);
                        tasks.push_back(std::make_pair(it->second->getArriveTime(), it->second->getCallBack()));
                    }
                } else {
                    break;
                }
            }
            // 从待处理事件队列中移除已处理的事件
            m_pending_events.erase(m_pending_events.begin(), it);
        }

        // 对于需要重复的定时事件，重新调整到达时间并重新添加到定时事件队列
        for (auto it = tmps.begin(); it != tmps.end(); ++it) {
            if ((*it)->isRepeated()) {
                (*it)->resetArriveTime();
                addTimerEvent(*it);
            }
        }

        // 重新设置定时器的到达时间
        resetArriveTime();

        // 执行所有到期的定时事件的回调函数
        for (auto task : tasks) {
            if (task.second) {
                task.second();
            }
        }
    }

    // 重置定时器的到期时间
    // 该函数根据当前定时事件队列中的最早到期时间重新设置定时器的到期时间。
    void Timer::resetArriveTime() {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto tmp = m_pending_events;
        lock.unlock();

        // 如果没有定时事件，则直接返回
        if (tmp.size() == 0) {
            return;
        }

        int64_t now = getNowMs();
        auto it = tmp.begin();
        int64_t interval = 0;

        // 计算下一个定时事件与当前时间的时间间隔
        if (it->second->getArriveTime() > now) {
            interval = it->second->getArriveTime() - now;
        } else {
            interval = 100; // 如果到期时间已经过期，则设置一个最小间隔
        }

        // 设置 timespec 结构体，包含秒和纳秒
        timespec ts;
        memset(&ts, 0, sizeof(ts));
        ts.tv_sec = interval / 1000;
        ts.tv_nsec = (interval % 1000) * 1000000;

        // 设置 itimerspec 结构体
        itimerspec value;
        memset(&value, 0, sizeof(value));
        value.it_value = ts;

        // 设置定时器超时定时
        // 当定时器超时时，它的文件描述符会变为可读状态，可以通过 epoll 或类似的I/O多路复用机制来检测这一状态
        // 在 Timer 类的 onTimer 方法中，这一点被用来触发和处理定时事件
        int rt = timerfd_settime(m_fd, 0, &value, nullptr);
        if (rt != 0) {
            ERRORLOG("timerfd_settime error, errno=%d, error=%s", errno, strerror(errno));
        }
        DEBUGLOG("timer reset to %lld", now + interval);
    }

}; // namespace netrpc