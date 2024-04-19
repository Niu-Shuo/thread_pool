#ifndef REACTOR_THREAD_POOL_DISPATCHER_H
#define REACTOR_THREAD_POOL_DISPATCHER_H

#include <string>
#include "Channel.h"

class EventLoop; // 先定义出，避免编译器在相互包含情况下出现蛋生鸡鸡生蛋问题

// 抽象类
// Epoll,Poll,Select模型
class Dispatcher
{
public:
    Dispatcher(struct EventLoop *eventLoop);
    virtual ~Dispatcher();

    // 添加，将某一结点添加到epoll树上
    virtual int add();

    // 删除,将某一个节点从epoll树上删除
    virtual int remove();

    // 修改
    virtual int modify();

    // 事件检测，用于检测epoll_wait等的一系列事件
    virtual int dispatch(int timeout = 2);
    inline void setChannel(Channel *channel)
    {
        m_channel = channel;
    }

protected:
    // std::string m_name = std::string();
    std::string m_name;     // 实例名字
    Channel *m_channel;     // channel
    EventLoop *m_eventLoop; // EventLoop
};

#endif