#ifndef REACTOR_THREAD_POOL_SELECTDISPATCHER_H
#define REACTOR_THREAD_POOL_SELECTDISPATCHER_H

#include "Dispatcher.h"

class SelectDispatcher : public Dispatcher
{
public:
    SelectDispatcher(struct EventLoop *eventLoop);
    ~SelectDispatcher();

    /*
        override修饰前面的函数，表示此函数是从父类继承过来的函数，子类将重写父类虚函数
        override会自动对前面的名字进行检查
    */
    // 添加，将某一个节点添加到epoll树上
    int add() override;

    // 删除，将某一个节点从epoll树上删除
    int remove() override;

    // 修改，将某一个节点从epoll树上修改
    int modify() override;

    // 事件检测， 用于检测待检测三者之一模型epoll_wait等的一系列事件上是否有事件被激活，读/写事件
    int dispatch(int timeout = 2) override; // 单位 S 超时时长
    // 不改变的不写，直接继承父类
private:
    void setFdSet();
    void clearFdSet();

private:
    fd_set m_readSet;           // 读事件集合
    fd_set m_writeSet;          // 写事件集合
    const int m_maxSize = 1024; // 最大容量
};

#endif