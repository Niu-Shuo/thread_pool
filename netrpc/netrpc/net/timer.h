/*
 * @Description: 定时器类声明 timer.h
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-05 19:46:08
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-05 19:54:12
 */
#ifndef NETRPC_NET_TIMER_H
#define NETRPC_NET_TIMER_H

#include <map>
#include <mutex>
#include "netrpc/net/fd_event.h"
#include "netrpc/net/timer_event.h"

namespace netrpc {

    // 定时器类，用于管理定时任务
    class Timer : public FdEvent {
    public:
        // 构造函数
        Timer();

        // 析构函数
        ~Timer();

        // 添加定时事件
        void addTimerEvent(TimerEvent::TimerEventPtr event);

        // 删除定时事件
        void deleteTimerEvent(TimerEvent::TimerEventPtr event);

        // 定时器触发回调函数，eventloop会执行这个回调函数
        void onTimer();

    private:
        // 重置定时器的到期时间
        void resetArriveTime();

    private:
        // 保存对应的时间和定时任务，multimap自动根据键（即到期时间）升序排序，最早到期的事件位于multimap的开始位置
        std::multimap<int64_t, TimerEvent::TimerEventPtr> m_pending_events;

        // 互斥锁，用于保护定时事件的添加和删除操作
        std::mutex m_mutex;
    };

}; // namespace netrpc

#endif // NETRPC_NET_TIMER_H