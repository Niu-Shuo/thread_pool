#ifndef REACTOR_THREAD_POOL_EVENTLOOP_H
#define REACTOR_THREAD_POOL_EVENTLOOP_H

#include "Channel.h"
#include <queue>
#include <map>
#include <thread>
#include <string>
#include <mutex>

// 处理该节点中Channel事件类型
enum class ElemType : char
{
    ADD,
    REMOVE,
    MODIFY
};

// 定义任务队列的节点 : 类型，文件描述符信息
struct ChannelElement
{
    ElemType type;
    Channel *channel;
};

class Dispatcher; // 先定义出，避免编译器在相互包含情况下出现蛋生鸡鸡生蛋问题

/*
    处理所有的事件，启动反应堆模型，处理机会文件描述符后的事件,添加任务，处理任务队列
    调用dispatcher中的添加移除，修改操作
    存储着任务队列m_taskQ  存储fd和对应channel对应关系:m_channelmap
    全局主线程->同时传入子线程
*/
class EventLoop
{
public:
    EventLoop();
    EventLoop(const std::string threadName);
    ~EventLoop();

    // 启动reactor模型
    int run();

    // 启动之后就会出现一些文件描述符需要处理
    // 处理激活的文件描述符fd,和激活的事件
    int eventActive(int fd, int event);

    // 添加任务到任务队列 ，添加任务队列可能存在同时访问，加互斥锁
    int addTask(Channel* channel, ElemType type);

    // 处理任务队列中的任务
    int processTaskQueue();

    // 添加：处理dispatcher中的节点
    // 把任务节点中的任务添加到dispatcher对应的检测集合里面
    int add(Channel* channel);

    // 移除：处理dispatcher中的节点
    int remove(Channel* channel);

    // 修改：处理dispatcher中的节点
    int modify(Channel* channel);

    // 释放channel需要的资源，释放channel，关掉socket，地址堆内存释放，channel和dispatcher的关系需要删除
    int freeChannel(Channel* channel);

    static int readLocalMessage(void*arg);
    int readMessage();

    // 返回线程ID
    inline std::thread::id getThreadID(){
        return m_threadID;
    }

    // 返回线程名
    inline std::string getThreadName(){
        return m_threadName;
    }
private:
    // 任务唤醒
    void taskWakeup();

private:
    bool m_isQuit;                            // 标志EventLoop是否工作
    Dispatcher *m_dispatcher;                 // 指向子类的实例select，poll，epoll
    std::queue<ChannelElement *> m_taskQueue; // 任务队列，存储任务，遍历任务队列就可以修改dispatcher检测的文件描述符
    std::map<int, Channel *> m_channelMap;    // 文件描述符和Channel之间的对应关系
    std::thread::id m_threadID;               // 线程ID
    std::string m_threadName;                 // 线程名
    std::mutex m_mutex;                       // 互斥锁，保护任务队列
    int m_socketPair[2];                      // 存储本地通信fd通过socketpair初始化
};
#endif