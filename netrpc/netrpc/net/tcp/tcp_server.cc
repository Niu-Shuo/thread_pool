/*
 * @Description: TcpServer 类的实现文件，负责管理 TCP 服务器的初始化、启动、连接管理等功能。
 * @Version: 1.0
 * @Author: Niu Shuo
 * @Date: 2024-06-13 21:16:08
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 21:35:18
 */
#include "netrpc/net/tcp/tcp_server.h"     // 包含 TcpServer 类的声明文件
#include "netrpc/net/eventloop.h"          // 包含事件循环的头文件
#include "netrpc/net/tcp/tcp_connection.h" // 包含 TCP 连接的头文件
#include "netrpc/common/log.h"             // 包含日志记录的头文件
#include "netrpc/common/config.h"          // 包含配置管理的头文件
#include "tcp_server.h"                    // 包含 TcpServer 类的头文件

namespace netrpc
{

    ZkClient TcpServer::zkCli; // 初始化静态成员变量

    TcpServer::TcpServer(NetAddr::NetAddrPtr local_addr) : m_local_addr(local_addr)
    {
        init(); // 初始化服务器
        INFOLOG("netrpc TcpServer listen success on [%s]", m_local_addr->toString().c_str());
    }

    TcpServer::~TcpServer()
    {
        // 销毁事件循环、IO 线程组和监听文件描述符事件
        if (m_main_eventloop)
        {
            delete m_main_eventloop;
            m_main_eventloop = nullptr;
        }
        if (m_io_thread_groups)
        {
            delete m_io_thread_groups;
            m_io_thread_groups = nullptr;
        }
        if (m_listen_fd_event)
        {
            delete m_listen_fd_event;
            m_listen_fd_event = nullptr;
        }
    }

    void TcpServer::start()
    {
        m_io_thread_groups->start(); // 启动 IO 线程组
        m_main_eventloop->loop();    // 启动主事件循环
    }

    void TcpServer::ClearClientTimerFunc()
    {
        auto it = m_client.begin();
        while (it != m_client.end())
        {
            if (*it != nullptr && (*it).use_count() > 0 && (*it)->getState() == Closed)
            {
                DEBUGLOG("TcpConnection [fd:%d] will be deleted, state=%d", (*it)->getFd(), (*it)->getState());
                it = m_client.erase(it); // 删除已关闭的客户端连接
            }
            else
            {
                ++it;
            }
        }
    }

    void TcpServer::init()
    {
        m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);                                                                 // 创建 TCP 连接接收器
        m_main_eventloop = EventLoop::GetCurrentEventLoop();                                                                      // 获取当前主事件循环
        m_io_thread_groups = new IOThreadGroup(Config::GetInst().m_io_threads);                                                   // 创建 IO 线程组
        m_listen_fd_event = new FdEvent(m_acceptor->getListenFd());                                                               // 创建监听文件描述符事件
        m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));                                      // 监听连接事件，绑定回调函数
        m_main_eventloop->addEpollEvent(m_listen_fd_event);                                                                       // 将监听事件添加到主事件循环中
        m_clear_client_timer_event = std::make_shared<TimerEvent>(5000, true, std::bind(&TcpServer::ClearClientTimerFunc, this)); // 创建清理客户端定时器事件
        m_main_eventloop->addTimerEvent(m_clear_client_timer_event);                                                              // 将定时器事件添加到主事件循环中
    }

    void TcpServer::onAccept()
    {
        auto re = m_acceptor->accept();            // 接受新的客户端连接
        int client_fd = re.first;                  // 获取客户端文件描述符
        NetAddr::NetAddrPtr peer_addr = re.second; // 获取客户端地址信息

        ++m_client_counts; // 客户端连接数加一

        auto io_thread = m_io_thread_groups->getIOThread();                                                                                               // 获取任意一个 IO 线程
        TcpConnection::TcpConnectionPtr connection = std::make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 128, peer_addr, m_local_addr); // 创建 TCP 连接对象
        connection->setState(Connected);                                                                                                                  // 设置连接状态为已连接

        m_client.insert(connection);                                        // 将连接添加到客户端连接集合中
        INFOLOG("TcpServer successfully accepts client, fd=%d", client_fd); // 记录成功接受客户端连接的日志
    }

}; // namespace netrpc