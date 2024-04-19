#include "EventLoop.h"
#include "EpollDispatcher.h"
#include "PollDispatcher.h"
#include "SelectDispatcher.h"
#include <string>
#include "Log.h"
#include <cstdlib>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>

EventLoop::EventLoop() : EventLoop(std::string())
{
}

EventLoop::EventLoop(const std::string threadName)
{

    m_isQuit = true;                                                          // 刚开始EventLoop没有运行
    m_threadID = std::this_thread::get_id();                                  // 当前的线程ID
    m_threadName = m_threadName == std::string() ? "MainThread" : threadName; // 子线程，主线程动态名
    m_dispatcher = new EpollDispatcher(this);                                 // 模型选择
    // m_dispatcher = new PollDispatcher(this);
    // m_dispatcher = new SelectDispatcher(this);
    m_channelMap.clear(); // 初始化清空

    // 线程通信socketpair初始化
    // eventLoop->socketPair传出参数，通信是两个，0发数据，通过1读出，反之相反
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_socketPair);

    if (ret == -1)
    {
        perror("socketpair失败!");
        exit(0);
    }
#if 0
    // 指定规则:evLoop->socketPair[0] 发送数据，evLoop->socketPair[1]接受数据
    // 接受数据添加到dispatcher检测文件描述符的集合中  ,readlocalMessage读事件对应的回调函数
    Channel* channel = new Channel(m_socketPair[1], FDEvent::ReadEvent,
        readlocalMessage, nullptr, nullptr, this);
#else
    // TODO 消化绑定 bind-绑定器，function类的成员函数无法对类的成员函数直接打包
    auto obj = std::bind(&EventLoop::readMessage, this);
    Channel *channel = new Channel(m_socketPair[1], FDEvent::ReadEvent,
                                   obj, nullptr, nullptr, this);
#endif
    // channel 添加到任务队列
    addTask(channel, ElemType::ADD);
}

EventLoop::~EventLoop()
{
}

int EventLoop::run()
{
    m_isQuit = false; // 启动
    // 比较线程ID，当前线程ID与我们保存的线程ID是否相等
    if (m_threadID != std::this_thread::get_id())
    {
        return -1; // 直接返回-1
    }

    // 循环进行事件处理
    while (!m_isQuit)
    {
        // 调用初始化时选中的模型Epoll,Poll，Select
        m_dispatcher->dispatch(); // 在工作时的超时时长
        processTaskQueue();       // 处理任务队列
    }
    return 0;
}

int EventLoop::eventActive(int fd, int event)
{
    // 判断传入的参数是否有效
    if(fd < 0){
        return -1;
    }
    
    return 0;
}

int EventLoop::addTask(Channel *channel, ElemType type)
{
    return 0;
}

int EventLoop::processTaskQueue()
{
    return 0;
}

int EventLoop::add(Channel *channel)
{
    return 0;
}

int EventLoop::remove(Channel *channel)
{
    return 0;
}

int EventLoop::modify(Channel *channel)
{
    return 0;
}

int EventLoop::freeChannel(Channel *channel)
{
    return 0;
}

int EventLoop::readLocalMessage(void *arg)
{
    return 0;
}

int EventLoop::readMessage()
{
    return 0;
}

void EventLoop::taskWakeup()
{
}
