/*
 * @Description: fd_event.cc epoll事件实现
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-05 19:12:38
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-14 21:49:35
 */

#include <fcntl.h>
#include "netrpc/common/log.h"
#include "netrpc/net/fd_event.h"

namespace netrpc
{

    // 构造函数，接受文件描述符并初始化监听事件结构体
    FdEvent::FdEvent(int fd) : m_fd(fd)
    {
        memset(&m_lisiten_events, 0, sizeof(m_lisiten_events)); // 初始化监听事件结构体
    }

    // 默认构造函数，初始化监听事件结构体
    FdEvent::FdEvent()
    {
        memset(&m_lisiten_events, 0, sizeof(m_lisiten_events)); // 初始化监听事件结构体
    }

    // 析构函数
    FdEvent::~FdEvent()
    {
    }

    // 将文件描述符设置为非阻塞模式
    void FdEvent::setNonBlock()
    {
        int flag = fcntl(m_fd, F_GETFL, 0); // 获取文件描述符的标志
        if (flag & O_NONBLOCK)
        { // 如果已是非阻塞，则返回
            return;
        }
        fcntl(m_fd, F_SETFL, flag | O_NONBLOCK); // 设置文件描述符为非阻塞
    }

    // 根据事件类型返回相应的回调函数
    std::function<void()> FdEvent::handler(TriggerEvent event_type)
    {
        if (event_type == TriggerEvent::IN_EVENT)
        {
            return m_read_callback; // 返回读事件回调函数
        }
        else if (event_type == TriggerEvent::OUT_EVENT)
        {
            return m_write_callback; // 返回写事件回调函数
        }
        else if (event_type == TriggerEvent::ERROR_EVENT)
        {
            return m_error_callback; // 返回错误事件回调函数
        }
        return nullptr; // 如果没有匹配的事件类型，返回空函数
    }

    // 注册监听事件及其回调函数
    void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback, std::function<void()> error_callback)
    {
        if (event_type == TriggerEvent::IN_EVENT)
        {
            m_lisiten_events.events |= EPOLLIN; // 注册读事件
            m_read_callback = callback;         // 设置读事件回调函数
        }
        else
        {
            m_lisiten_events.events |= EPOLLOUT; // 注册写事件
            m_write_callback = callback;         // 设置写事件回调函数
        }

        if (m_error_callback == nullptr)
        {
            m_error_callback = error_callback; // 设置错误事件回调函数
        }
        else
        {
            m_error_callback = nullptr; // 重置错误事件回调函数
        }

        m_lisiten_events.data.ptr = this; // 将当前对象指针保存到监听事件结构体中
    }

    // 取消监听事件
    void FdEvent::cancle(TriggerEvent event_type)
    {
        if (event_type == TriggerEvent::IN_EVENT)
        {
            m_lisiten_events.events &= (~EPOLLIN); // 取消读事件监听
        }
        else
        {
            m_lisiten_events.events &= (~EPOLLOUT); // 取消写事件监听
        }
    }

}; // namespace netrpc
