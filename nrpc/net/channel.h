#ifndef NRPC_NET_CHANNEL_H
#define NRPC_NET_CHANNEL_H

#include <unistd.h>

namespace nrpc
{

    // 前置声明 EventLoop 类，以避免循环包含
    class EventLoop;

    // Channel 类
    class Channel
    {
    public:
        // 构造函数，初始化 Channel 对象
        Channel(int fd = -1, EventLoop *elp = nullptr) : m_fd(fd), m_event(0), m_sevent(0), m_elp(elp) {}

        // 析构函数，关闭文件描述符
        virtual ~Channel()
        {
            if (m_fd != -1)
            {
                ::close(m_fd);
                m_fd = -1;
            }
        }

        // 获取文件描述符
        int fd() const { return m_fd; }

        // 获取事件循环指针
        EventLoop *elp() const { return m_elp; }

        // 获取当前关注的事件
        int event() const { return m_event; }

        // 获取触发的事件
        int sevent() const { return m_sevent; }

        // 设置关注的事件
        void set_event(int event) { m_event = event; }

        // 设置触发的事件
        void set_sevent(int sevent) { m_sevent = sevent; }

        // 处理事件的纯虚函数，需要在派生类中实现
        virtual void handle_events(int events) = 0;

    protected:
        int m_fd;         // 文件描述符
        int m_event;      // 关注的事件
        int m_sevent;     // 触发的事件
        EventLoop *m_elp; // 关联的事件循环指针
    };

} // namespace nrpc

#endif