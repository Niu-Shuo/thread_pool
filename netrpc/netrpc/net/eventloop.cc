/*
 * @Description: 事件循环处理类实现
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-05 21:00:09
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-15 21:44:54
 */
#include <sys/eventfd.h>
#include "netrpc/net/eventloop.h"
#include "netrpc/common/log.h"
#include "netrpc/common/util.h"

#define ADD_TO_EPOLL()                                                                        \
    auto it = m_listen_fds.find(event->getFd());                                              \
    int op = EPOLL_CTL_ADD;                                                                   \
    if (it != m_listen_fds.end())                                                             \
    {                                                                                         \
        op = EPOLL_CTL_MOD;                                                                   \
    }                                                                                         \
    epoll_event tmp = event->getEpollEvent();                                                 \
    INFOLOG("epoll_event.events = %d", (int)tmp.events);                                      \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);                                 \
    if (rt == -1)                                                                             \
    {                                                                                         \
        ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    }                                                                                         \
    m_listen_fds.insert(event->getFd());                                                      \
    DEBUGLOG("add event success, fd[%d]", event->getFd())

#define DELETE_TO_EPOLL()                                                                     \
    auto it = m_listen_fds.find(event->getFd());                                              \
    if (it == m_listen_fds.end())                                                             \
    {                                                                                         \
        return;                                                                               \
    }                                                                                         \
    int op = EPOLL_CTL_DEL;                                                                   \
    epoll_event tmp = event->getEpollEvent();                                                 \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), nullptr);                                 \
    if (rt == -1)                                                                             \
    {                                                                                         \
        ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    }                                                                                         \
    m_listen_fds.erase(event->getFd());                                                       \
    DEBUGLOG("delete event success, fd[%d]", event->getFd());

namespace netrpc
{

    // 线程局部变量，用于存储当前线程的 EventLoop 实例
    static thread_local EventLoop *t_current_eventloop = nullptr;
    // 全局变量，epoll 的最大超时时间和最大事件数
    static int g_epoll_max_timeout = 10000;
    static int g_epoll_max_events = 10;

    // 构造函数，初始化事件循环
    EventLoop::EventLoop()
    {
        if (t_current_eventloop != nullptr)
        {
            ERRORLOG("failed to create event loop, this thread has create event loop");
            exit(0);
        }
        m_thread_id = getThreadId();    // 获取当前线程 ID
        m_epoll_fd = epoll_create1(10); // 创建 epoll 文件描述符

        if (m_epoll_fd == -1)
        {
            ERRORLOG("failed to create event loop, epoll_create error, error info[%d]", errno);
            exit(0);
        }
        initWakeUpFdEvent(); // 初始化唤醒文件描述符事件
        initTimer();         // 初始化定时器
        INFOLOG("succ create event loop in the thread %d", m_thread_id);
        t_current_eventloop = this; // 设置当前线程的 EventLoop 实例
    }

    // 析构函数
    EventLoop::~EventLoop()
    {
        close(m_epoll_fd);
        if (m_wakeup_fd_event) {
            delete m_wakeup_fd_event;
            m_wakeup_fd_event = nullptr;
        }

        if (m_timer) {
            delete m_timer;
            m_timer = nullptr;
        }
    }

    // 事件循环的主函数
    // 该函数实现了事件循环机制，不断地处理待处理的任务和监听文件描述符的事件。
    void EventLoop::loop() {
        m_is_looping = true;  // 标记事件循环开始
        while (!m_quit) {  // 当没有接收到退出信号时，持续运行循环
            std::unique_lock<std::mutex> lock(m_mutex);
            std::queue<std::function<void()>> tmp_tasks;
            m_pending_tasks.swap(tmp_tasks);  // 交换当前待处理任务队列和临时队列
            lock.unlock();

            // 处理所有待处理的任务
            while (!tmp_tasks.empty()) {
                std::function<void()> cb = tmp_tasks.front();  // 获取队列中的第一个任务
                tmp_tasks.pop();  // 将任务从队列中移除
                if (cb) {  // 如果任务不为空，执行任务
                    cb();
                }
            }

            int timeout = g_epoll_max_timeout;  // 设置 epoll 的超时时间
            epoll_event result_events[g_epoll_max_events];
            // 调用 epoll 等待事件发生，超时事件设为 timeout
            int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout);
            DEBUGLOG("now end epoll_wait, rt = %d", rt);

            if (rt < 0) {  // 如果 epoll_wait 调用失败，记录错误日志
                ERRORLOG("epoll_wait error, errno=%d, error=%s", errno, strerror(errno));
            } else {
                // 遍历所有触发的事件
                for (int ii = 0; ii < rt; ++ii) {
                    epoll_event trigger_event = result_events[ii];
                    FdEvent *fd_event = static_cast<FdEvent *>(trigger_event.data.ptr);
                    if (fd_event == nullptr) {
                        ERRORLOG("fd_event = nullptr, continue");
                        continue;
                    }

                    // 处理 EPOLLIN 事件
                    if (trigger_event.events & EPOLLIN) {
                        DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::IN_EVENT));  // 将 EPOLLIN 事件任务添加到任务队列
                    }
                    // 处理 EPOLLOUT 事件
                    if (trigger_event.events & EPOLLOUT) {
                        DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::OUT_EVENT));  // 将 EPOLLOUT 事件任务添加到任务队列
                    }

                    // 处理 EPOLLERR 事件
                    if (trigger_event.events & EPOLLERR) {
                        DEBUGLOG("fd %d trigger EPOLLERROR event", fd_event->getFd());
                        deleteEpollEvent(fd_event);  // 删除出现错误的事件
                        if (fd_event->handler(FdEvent::ERROR_EVENT) != nullptr) {
                            DEBUGLOG("fd %d add error callback", fd_event->getFd());
                            addTask(fd_event->handler(FdEvent::ERROR_EVENT));  // 将 EPOLLERR 事件任务添加到任务队列
                        }
                    }
                }
            }
        }
    }

    // 唤醒事件循环
    void EventLoop::wakeup()
    {
        INFOLOG("WAKE UP");
        m_wakeup_fd_event->wakeup();
    }

    // 停止事件循环
    void EventLoop::stop()
    {
        m_quit = true;
        wakeup();
    }

    // 添加事件到 epoll
    void EventLoop::addEpollEvent(FdEvent *event)
    {
        if (isInLoopThread())
        {
            ADD_TO_EPOLL();
        }
        else
        {
            auto cb = [this, event]()
            {
                ADD_TO_EPOLL();
            };
            addTask(cb, true);
        }
    }

    // 从 epoll 删除事件
    void EventLoop::deleteEpollEvent(FdEvent *event)
    {
        if (isInLoopThread())
        {
            DELETE_TO_EPOLL();
        }
        else
        {
            auto cb = [this, event]()
            {
                DELETE_TO_EPOLL();
            };
            addTask(cb, true);
        }
    }

    // 判断是否在事件循环的线程中
    bool EventLoop::isInLoopThread()
    {
        return getThreadId() == m_thread_id;
    }

    // 添加任务到任务队列
    void EventLoop::addTask(std::function<void()> cb, bool is_wake_up)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_pending_tasks.push(cb);
        }
        if (is_wake_up)
        {
            wakeup();
        }
    }

    // 添加定时器事件
    void EventLoop::addTimerEvent(TimerEvent::TimerEventPtr event)
    {
        m_timer->addTimerEvent(event);
    }

    // 判断事件循环是否正在运行
    bool EventLoop::isLooping()
    {
        return m_is_looping;
    }

    // 获取当前线程的事件循环实例
    EventLoop *EventLoop::GetCurrentEventLoop()
    {
        if (t_current_eventloop)
        {
            return t_current_eventloop;
        }
        t_current_eventloop = new EventLoop();
        return t_current_eventloop;
    }

    // 处理唤醒事件
    void EventLoop::dealWakeup()
    {
        std::queue<std::function<void()>> tmp_wakeup_tasks;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_pending_tasks.swap(tmp_wakeup_tasks);
        }
        while (!tmp_wakeup_tasks.empty())
        {
            auto task = tmp_wakeup_tasks.front();
            tmp_wakeup_tasks.pop();
            task();
        }
    }

    // 初始化唤醒文件描述符事件
    void EventLoop::initWakeUpFdEvent()
    {
        m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
        if (m_wakeup_fd < 0)
        {
            ERRORLOG("failed to create event loop, eventfd create error, error info[%d]", errno);
            exit(0);
        }
        INFOLOG("wakeup fd = %d", m_wakeup_fd);
        m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
        m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this]()
                                  {
            char buf[8];
            while (read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN) {}
            DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd); });
        addEpollEvent(m_wakeup_fd_event);
    }

    // 初始化定时器
    void EventLoop::initTimer()
    {
        m_timer = new Timer();  // 创建新的定时器
        addEpollEvent(m_timer); // 添加定时器事件到 epoll
    }
};