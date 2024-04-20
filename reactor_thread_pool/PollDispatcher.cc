#include "PollDispatcher.h"
#include "EventLoop.h"
#include <poll.h>
#include <cstdlib>
#include <unistd.h>
#include "Log.h"

PollDispatcher::PollDispatcher(EventLoop *eventLoop) : Dispatcher(eventLoop)
{
    m_maxfd = 0;
    m_fds = new struct pollfd[m_maxNode];

    // 初始化
    for (int ii = 0; ii != m_maxNode; ++ii)
    {
        m_fds[ii].fd = -1;     // 初始化-1为无效的文件描述符
        m_fds[ii].events = 0;  // 当前文件描述符检测的事件
        m_fds[ii].revents = 0; // 返回的事件
    }
    m_name = "Poll";
}

PollDispatcher::~PollDispatcher()
{
    delete[] m_fds;
}

int PollDispatcher::add()
{
    int events = 0;
    if (m_channel->getEvent() & static_cast<int>(FDEvent::ReadEvent))
    {
        events |= POLLIN; // 保存相应的读事件
    }
    if (m_channel->getEvent() & static_cast<int>(FDEvent::WriteEvent))
    {
        events |= POLLOUT; // 保存相应的写事件
    }

    // 找出空闲的可用元素
    int ii;
    for (ii = 0; ii < m_maxNode; ++ii)
    {
        if (m_fds[ii].fd == -1)
        {
            m_fds[ii].events = events;             // 事件
            m_fds[ii].fd = m_channel->getSocket(); // 文件描述符
            m_maxfd = std::max(m_maxfd, ii);
            break;
        }
    }

    if (ii >= m_maxNode)
    {
        return -1;
    }

    return 0;
}

int PollDispatcher::remove()
{
    int ii;
    for (ii = 0; ii != m_maxNode; ++ii)
    {
        if (m_fds[ii].fd == m_channel->getSocket())
        {
            m_fds[ii].events = 0;  // 当前文件描述符检测的事件
            m_fds[ii].revents = 0; // 返回的事件
            m_fds[ii].fd = -1;     // 赋值-1为无效的文件描述符
            break;
        }
    }

    // 通过channel释放对应的TcpConnection资源
    // arg为 struct TcpConnection* conn = (struct TcpConnection*)arg;
    m_channel->destroyCallback(const_cast<void *>(m_channel->getArg()));
    if (ii >= m_maxNode)
    {
        return -1;
    }
    return 0;
}

int PollDispatcher::modify()
{
    int events = 0;
    if (m_channel->getEvent() & static_cast<int>(FDEvent::ReadEvent))
    {
        events |= POLLIN; // 保存相应的读事件
    }
    if (m_channel->getEvent() & static_cast<int>(FDEvent::WriteEvent))
    {
        events |= POLLOUT; // 保存相应的写事件
    }

    // 空闲可用元素
    int ii;
    for (int ii = 0; ii != m_maxNode; ++ii)
    {
        if (m_fds[ii].fd == m_channel->getSocket())
        {
            m_fds[ii].events = events;
            break;
        }
    }
    if (ii >= m_maxNode)
    {
        return -1;
    }
    return 0;
}

int PollDispatcher::dispatch(int timeout)
{
    // 第二个参数 内存需要根据此参数确定遍历数组的时候的范围
    int count = poll(m_fds, m_maxfd + 1, timeout * 1000); // 毫秒
    if (count == -1)
    {
        perror("Poll失败!");
        exit(0);
    }

    for (int ii = 0; ii != m_maxNode; ++ii)
    {
        if (m_fds->fd == -1)
        {
            continue;
        }
        if (m_fds[ii].revents & POLLIN)
        {
            m_eventLoop->eventActive(m_fds[ii].fd, static_cast<int>(FDEvent::ReadEvent)); // 读事件
        }
        if (m_fds[ii].revents & POLLOUT)
        {
            m_eventLoop->eventActive(m_fds[ii].fd, static_cast<int>(FDEvent::WriteEvent)); // 写事件
        }
    }
    return 0;
}
