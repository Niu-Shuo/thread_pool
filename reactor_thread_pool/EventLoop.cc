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
#include <string.h>

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
    if (fd < 0)
    {
        return -1;
    }

    // 基于fd从对应的EventLoop中取出Channel
    Channel *channel = m_channelMap[fd];

    // 判断取出的channel的fd与当前fd是否相同
    assert(channel->getSocket() == fd); // 如果为假，打印出报错信息

    // 读事件，且读回调不为空
    if (event & static_cast<int>(FDEvent::ReadEvent) && channel->readCallback)
    {
        // 调用channel的读回调函数
        channel->readCallback(const_cast<void *>(channel->getArg()));
    }

    // 写事件，且写回调不为空
    if (event & static_cast<int>(FDEvent::WriteEvent) && channel->writeCallback)
    {
        // 调用channel的写回调函数
        channel->writeCallback(const_cast<void *>(channel->getArg()));
    }

    return 0;
}

int EventLoop::addTask(Channel *channel, ElemType type)
{
    // 加锁，有可能是当前线程，也有可能是主线程
    m_mutex.lock();

    // 创建新节点
    ChannelElement *node = new ChannelElement;
    node->channel = channel;
    node->type = type;
    m_taskQueue.push(node);
    m_mutex.unlock();

    // 处理节点
    /*
     * 如当前EventLoop反应堆属于子线程
     *   1，对于链表节点的添加：可能是当前线程也可能是其它线程(主线程)
     *       1),修改fd的事件，可能是当前线程发起的，还是当前子线程进行处理
     *       2),添加新的fd，和新的客户端发起连接，添加任务节点的操作由主线程发起
     *   2，主线程只负责和客户端建立连接，判断当前线程，不让主线程进行处理，分给子线程
     *       不能让主线程处理任务队列，需要由当前的子线程处理
     */
    if (m_threadID == std::this_thread::get_id())
    {
        // 当前子线程，直接处理任务
        processTaskQueue();
    }
    else
    {
        /*
            主线程 -- 告诉子线程处理任务队列中的任务
            1,子线程在工作
            2，子线程被阻塞了：1，select,poll,epoll,如何解除其阻塞，在本代码阻塞时长是2s
             在检测集合中添加属于自己(额外)的文件描述，不负责套接字通信，目的控制文件描述符什么时候有数据,辅助解除阻塞
             满足条件，两个文件描述符，可以相互通信，
            1，使用pipe进程间通信，进程更可
            2，socketpair 文件描述符进行通信
        */
        taskWakeup(); // 主线程调用，相当于向socket添加了数据
    }
    return 0;
}

int EventLoop::processTaskQueue()
{
    // 遍历任务队列
    while (!m_taskQueue.empty())
    {
        // 将处理后的task从当前链表中删除，(需要加锁)
        // 取出头结点
        m_mutex.lock();
        ChannelElement *node = m_taskQueue.front();
        m_taskQueue.pop();
        m_mutex.unlock();

        // 读取并处理Channel
        Channel *channel = node->channel;
        if (node->type == ElemType::ADD)
        {
            // 需要channel里面的文件描述符eventLoop里面的数据
            // 添加  -- 每个功能对应一个任务函数，更利于维护
            add(channel);
        }
        else if (node->type == ElemType::REMOVE)
        {
            // Debug("断开了连接");
            // 删除
            remove(channel);
            // 需要资源释放channel 关掉文件描述符，地址堆内存释放，channel和dispatcher的关系需要删除
        }
        else if (node->type == ElemType::MODIFY)
        {
            // 修改
            modify(channel);
        }
        delete node;
    }
    return 0;
}

int EventLoop::add(Channel *channel)
{
    // 把任务节点中的任务添加到dispatcher对应的检测集合里面
    int fd = channel->getSocket();

    // 找到fd对应数组元素的位置，并存储
    if (m_channelMap.find(fd) == m_channelMap.end())
    {
        m_channelMap.insert(std::make_pair(fd, channel)); // 将当前fd和channel添加到map
        m_dispatcher->setChannel(channel);                // 设置当前channel
        int ret = m_dispatcher->add();
        return ret;
    }

    return -1;
}

int EventLoop::remove(Channel *channel)
{
    // 调用dispatcher的remove函数进行删除
    // 将要删除的文件描述符
    int fd = channel->getSocket();

    // 判断文件描述符是否已经在检测的集合了
    if (m_channelMap.find(fd) == m_channelMap.end())
    {
        return -1;
    }

    // 从集合中删除
    m_dispatcher->setChannel(channel);
    int ret = m_dispatcher->remove();

    return ret;
}

int EventLoop::modify(Channel *channel)
{
    // 将要修改的文件描述符
    int fd = channel->getSocket();

    // 判断
    if (m_channelMap.find(fd) == m_channelMap.end())
    {
        return -1;
    }

    // 在集合中修改
    m_dispatcher->setChannel(channel);
    int ret = m_dispatcher->modify();

    return ret;
}

int EventLoop::freeChannel(Channel *channel)
{
    // 删除channel 和fd的对应关系
    auto it = m_channelMap.find(channel->getSocket());
    if (it != m_channelMap.end())
    {
        m_channelMap.erase(it);      // 利用迭代器删除
        close(channel->getSocket()); // 关闭套接字
        delete channel;
    }
    return 0;
}

int EventLoop::readLocalMessage(void *arg)
{
    EventLoop *eventLoop = static_cast<EventLoop *>(arg);
    char buf[256];
    read(eventLoop->m_socketPair[1], buf, sizeof(buf)); // 目的仅为触发一次读事件，检测文件描述符解除阻塞
    return 0;
}

int EventLoop::readMessage()
{
    char buf[256];
    read(m_socketPair[1], buf, sizeof(buf)); // 目的仅为触发一次读事件，检测文件描述符解除阻塞
    return 0;
}

void EventLoop::taskWakeup()
{
    const char* msg = "唤醒线程!";
    write(m_socketPair[0], msg, strlen(msg));
}
