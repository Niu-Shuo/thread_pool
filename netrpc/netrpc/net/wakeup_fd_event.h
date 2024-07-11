/*
 * @Description: 唤醒时间类 wakeup_fd_event.h
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-05 20:48:51
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-05 20:55:41
 */
#ifndef NETRPC_NET_WAKEUP_FDEVENT_H
#define NETRPC_NET_WAKEUP_FDEVENT_H

#include "netrpc/net/fd_event.h"

namespace netrpc
{

    // WakeUpFdEvent 类继承自 FdEvent 类，用于处理唤醒事件
    class WakeUpFdEvent : public FdEvent
    {
    public:
        // 构造函数，接受一个文件描述符作为参数
        WakeUpFdEvent(int fd);

        // 析构函数
        ~WakeUpFdEvent();

        // 唤醒函数
        void wakeup();
    };
};

#endif