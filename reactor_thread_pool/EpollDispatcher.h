#ifndef REACTOR_THREAD_POOL_EPOLLDISPATCHER_H
#define REACTOR_THREAD_POOL_EPOLLDISPATCHER_H

#include "Dispatcher.h"

class EpollDispatcher : public Dispatcher{  // 继承父类Dispatcher
public:
    EpollDispatcher(struct EventLoop* eventLoop);
    ~EpollDispatcher();

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
    int epollCtl(int op);

private:
    int m_epfd; // epoll的根节点
    struct epoll_event* m_events;
    const int m_maxNode = 520;
};

#endif