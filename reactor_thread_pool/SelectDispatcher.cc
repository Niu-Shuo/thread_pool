#include "SelectDispatcher.h"
#include <cstdlib>
#include <unistd.h>
#include "EventLoop.h"
#include "Log.h"

SelectDispatcher::SelectDispatcher(EventLoop *eventLoop) : Dispatcher(eventLoop)
{
    // 对集合进行清空，所有标志位设置为0
    FD_ZERO(&m_readSet);
    FD_ZERO(&m_writeSet);
    m_name = "Select";
}

SelectDispatcher::~SelectDispatcher()
{
}

int SelectDispatcher::add()
{
    if (m_channel->getSocket() >= m_maxSize)
    {
        return -1; // 最大1024
    }
    setFdSet();
    return 0;
}

int SelectDispatcher::remove()
{
    clearFdSet();
    // 通过channel释放对应的TcpConnection资源
    // arg为 struct TcpConnection* conn = (struct TcpConnection*)arg;
    m_channel->destroyCallback(const_cast<void *>(m_channel->getArg()));
    return 0;
}

int SelectDispatcher::modify()
{
    setFdSet();
    clearFdSet();
    return 0;
}

int SelectDispatcher::dispatch(int timeout)
{
    struct timeval val;
    val.tv_sec = timeout; // 秒
    val.tv_usec = 0;      // 微妙,初始化一下,防止随机数

    // 234传入传出参数，内核会修改传入数据，原始数据传给内核之后，数据越来越少，出现丢失现象，
    // 对原始数据进行备份
    fd_set rdtmp = m_readSet;
    fd_set wrtmp = m_writeSet;

    int count = select(m_maxSize, &rdtmp, &wrtmp, NULL, &val);
    if (count == -1)
    {
        perror("Select失败!");
        exit(0);
    }

    for (int ii = 0; ii != m_maxSize; ++ii)
    {
        if (FD_ISSET(ii, &rdtmp))
        {                                                                       // 读事件被激活
            m_eventLoop->eventActive(ii, static_cast<int>(FDEvent::ReadEvent)); // 读事件
        }
        if (FD_ISSET(ii, &wrtmp))
        {                                                                        // 写事件被激活
            m_eventLoop->eventActive(ii, static_cast<int>(FDEvent::WriteEvent)); // 写事件
        }
    }
    return 0;
}

void SelectDispatcher::setFdSet()
{
    if(m_channel->getEvent() & static_cast<int>(FDEvent::ReadEvent)){
        // 保存相应的读事件
        // 添加到对应的读写事件内
        FD_SET(m_channel->getSocket(), &m_readSet);
    }
    if(m_channel->getEvent() & static_cast<int>(FDEvent::WriteEvent)){
        // 保存相应的写事件
        // 添加到对应的读写事件内
        FD_SET(m_channel->getSocket(), &m_writeSet);
    }
}

void SelectDispatcher::clearFdSet()
{
    if(m_channel->getEvent() & static_cast<int>(FDEvent::ReadEvent)){
        // 清除相应的读事件
        FD_CLR(m_channel->getSocket(), &m_readSet);
    }

    if(m_channel->getEvent() & static_cast<int>(FDEvent::WriteEvent)){
        // 清除相应的写事件
        FD_CLR(m_channel->getSocket(), &m_writeSet);
    }
}
