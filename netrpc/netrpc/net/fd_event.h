/*
 * @Description: epoll逻辑 fd_event.h
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-05 18:58:06
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-05 22:29:43
 */
#ifndef NETRPC_NET_FDEVENT_H
#define NETRPC_NET_FDEVENT_H

#include <sys/epoll.h>
#include <functional>

namespace netrpc
{

    // FdEvent类用于封装文件描述符事件的处理
    class FdEvent
    {
    public:
        // 定义触发事件的枚举
        enum TriggerEvent
        {
            IN_EVENT = EPOLLIN,     // 可读事件
            OUT_EVENT = EPOLLOUT,   // 可写事件
            ERROR_EVENT = EPOLLERR, // 错误事件
        };

        // 构造函数，接受文件描述符
        FdEvent(int fd);

        // 默认构造函数
        FdEvent();

        // 析构函数
        ~FdEvent();

        // 将对应的文件描述符设置为非阻塞模式
        void setNonBlock();

        // 根据事件类型返回对应的回调函数
        std::function<void()> handler(TriggerEvent event_type);

        // 注册监听事件及其回调函数
        void listen(TriggerEvent event_type, std::function<void()> callback, std::function<void()> error_callback = nullptr);

        // 取消监听事件
        void cancle(TriggerEvent event_type);

        // 获取文件描述符
        int getFd() const
        {
            return m_fd;
        }

        // 获取epoll_event结构体
        epoll_event getEpollEvent()
        {
            return m_lisiten_events;
        }

    protected:
        int m_fd{-1};                                    // 文件描述符，初始化为-1
        epoll_event m_lisiten_events;                    // epoll_event结构体，保存监听的事件
        std::function<void()> m_read_callback{nullptr};  // 读事件回调函数
        std::function<void()> m_write_callback{nullptr}; // 写事件回调函数
        std::function<void()> m_error_callback{nullptr}; // 错误事件回调函数
    };

}; // namespace netrpc

#endif // NETRPC_NET_FDEVENT_H