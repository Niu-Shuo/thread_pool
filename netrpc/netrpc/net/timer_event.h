/*
 * @Description: Timer_event.h 定时事件类声明
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 18:48:37
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-14 22:11:26
 */
#ifndef NETRPC_NET_TIMEREVENT_H
#define NETRPC_NET_TIMEREVENT_H

#include <functional>
#include <memory>

namespace netrpc
{

    // 定时事件类，用于管理定时任务
    class TimerEvent
    {
    public:
        using TimerEventPtr = std::shared_ptr<TimerEvent>;

        // 构造函数，接受时间间隔、是否重复执行和回调函数
        TimerEvent(int interval, bool is_repeated, std::function<void()> cb);

        // 获取定时事件的到达时间
        int64_t getArriveTime() const
        {
            return m_arrive_time;
        }

        // 设置定时事件是否取消
        void setCancled(bool value)
        {
            m_is_cancled = value;
        }

        // 检查定时事件是否已取消
        bool isCancled() const
        {
            return m_is_cancled;
        }

        // 检查定时事件是否重复执行
        bool isRepeated() const
        {
            return m_is_repeated;
        }

        // 获取定时事件的回调函数
        std::function<void()> getCallBack() const
        {
            return m_task;
        }

        // 重置定时事件的到达时间
        void resetArriveTime();

    private:
        int64_t m_arrive_time;        // 定时事件的到达时间
        int64_t m_interval;           // 定时事件的时间间隔
        bool m_is_repeated{false};    // 定时事件是否重复执行
        bool m_is_cancled{false};     // 定时事件是否已取消
        std::function<void()> m_task; // 定时事件的回调函数
    };

}

#endif
