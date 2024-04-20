#include "Channel.h"

Channel::Channel(int fd, FDEvent events, handleFunc readFunc, handleFunc writeFunc, handleFunc destoryFunc, void *arg)
{
    // 类初始化
    m_fd = fd;
    // m_events = (int)events;
    m_events = static_cast<int>(events);
    m_arg = arg;

    readCallback = readFunc;
    writeCallback = writeFunc;
    destroyCallback = destoryFunc;
}

/*
    添加写属性
    若对应为10 想要写添加写属性，与100异或，的110读写属性
    如不写，第三位清零，若为110，第三位清零，将写取反011，在按位与& 010只留下读事件
*/
void Channel::writeEventEnable(bool flag)
{
    // 添加写事件
    if (flag)
    {
        m_events |= static_cast<int>(FDEvent::WriteEvent); // WriteEvent 从右往左数第三个标志位1，通过异或 让channel->events的第三位为1
    }
    else
    {
        // ~WriteEvent 按位与， ~WriteEvent取反 011 然后与 channel->events按位与&运算 只有11 为 1，其它皆为0只有同为真时则真，一假则假，1为真，0为假
        // m_events = m_events & ~static_cast<int>(FDEvent::WriteEvent);
        m_events &= ~static_cast<int>(FDEvent::WriteEvent); // channel->events 第三位清零之后，写事件就不再检测
    }
}

bool Channel::isWriteEventEnable()
{
    return m_events & static_cast<int>(FDEvent::WriteEvent); // 按位与 ，第三位都是1，则是写，如果成立，最后大于0，如果不成立，最后为0
}

void Channel::readEventEnable(bool flag)
{
    // 添加写事件
    if (flag)
    {
        m_events |= static_cast<int>(FDEvent::ReadEvent); // ReadEvent 从右往左数第二个标志位1，通过异或 让channel->events的第二位为1
    }
    else
    {
        // ~ReadEvent 按位与， ~ReadEvent取反 011 然后与 channel->events按位与&运算 只有11 为 1，其它皆为0只有同为真时则真，一假则假，1为真，0为假
        // m_events = m_events & ~static_cast<int>(FDEvent::ReadEvent);
        m_events &= ~static_cast<int>(FDEvent::ReadEvent); // channel->events 第二位清零之后，写事件就不再检测
    }
}

bool Channel::isReadEventEnable()
{
    return m_events & static_cast<int>(FDEvent::ReadEvent);
}

/*
void Channel::timeoutEventEnable(bool flag)
{
    // 添加写事件
    if (flag)
    {
        m_events |= static_cast<int>(FDEvent::TimeOut); // TimeOut 从右往左数第一个标志位1，通过异或 让channel->events的第一位为1
    }
    else
    {
        // ~TimeOut 按位与， ~TimeOut取反 011 然后与 channel->events按位与&运算 只有11 为 1，其它皆为0只有同为真时则真，一假则假，1为真，0为假
        // m_events = m_events & ~static_cast<int>(FDEvent::TimeOut);
        m_events &= ~static_cast<int>(FDEvent::TimeOut); // channel->events 第一位清零之后，超时事件就不再检测
    }
}
bool Channel::isTimeoutEventEnable()
{
    return false;
}
*/
