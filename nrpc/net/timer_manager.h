#ifndef NRPC_NET_TIMER_MANAGER_H
#define NRPC_NET_TIMER_MANAGER_H

#include <queue>
#include <functional>
#include "base/timestamp.h"

namespace nrpc
{
    // 定时器管理类
    class TimerManager
    {
    public:
        // 定时器类
        class Timer
        {
        public:
            // 构造函数，传入时间戳和回调函数
            Timer(const TimeStamp &ts, const std::function<void()> &func) : m_data(std::make_pair(ts, func)) {}

            // 重载小于运算符，用于小顶堆排序
            bool operator<(const Timer &that) const
            {
                return !(m_data.first < that.m_data.first);
            }

            // 获取时间戳
            const TimeStamp &timestamp() const
            {
                return m_data.first;
            }

            // 获取回调函数
            std::function<void()> &func()
            {
                return m_data.second;
            }

        private:
            std::pair<nrpc::TimeStamp, std::function<void()>> m_data;
            friend class TimerManager;
        };

    public:
        // 判断定时器队列是否为空
        bool empty() const
        {
            return m_queue.empty();
        }

        // 获取队首定时器
        const Timer &top()
        {
            return m_queue.top();
        }

        // 弹出队首定时器
        void pop()
        {
            if (!m_queue.empty())
            {
                m_queue.pop();
            }
        }

        // 获取最近的定时器的时间戳
        TimeStamp get_latest_timer() const
        {
            if (m_queue.empty())
            {
                return TimeStamp();
            }
            const Timer &tm = m_queue.top();
            return tm.timestamp();
        }

        // 添加定时器
        void add_timer(const TimeStamp &ts, const std::function<void()> &func)
        {
            m_queue.emplace(ts, func);
        }
        // 不提供删除定时器的接口，定时器是否有效由用户回调函数自行判断
        // void remove_timer();
    private:
        std::priority_queue<Timer> m_queue; // 优先队列用于存储定时器
    };
}; // namespace nrpc

#endif