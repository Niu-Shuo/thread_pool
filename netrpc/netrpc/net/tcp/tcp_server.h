/*
 * @Description: TcpServer 类用于管理 TCP 服务器，包括处理连接、事件循环和定时任务等功能。
 * @Version: 1.0
 * @Author: Niu Shuo
 * @Date: 2024-06-13 21:10:07
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 21:15:51
 */
#ifndef NETRPC_NET_TCP_TCPSERVER_H
#define NETRPC_NET_TCP_TCPSERVER_H

#include <set>
#include "netrpc/net/tcp/tcp_acceptor.h"   // 包含 TCP 连接接收器的头文件
#include "netrpc/net/tcp/net_addr.h"       // 包含网络地址的头文件
#include "netrpc/net/eventloop.h"          // 包含事件循环的头文件
#include "netrpc/net/io_thread_group.h"    // 包含 IO 线程组的头文件
#include "netrpc/net/tcp/tcp_connection.h" // 包含 TCP 连接的头文件
#include "netrpc/common/zookeeperutil.h"   // 包含 Zookeeper 工具类的头文件

namespace netrpc
{

    class TcpServer
    {
    public:
        // 构造函数，指定本地监听地址
        TcpServer(NetAddr::NetAddrPtr local_addr);

        // 析构函数
        ~TcpServer();

        // 启动服务器，开始监听并接受连接
        void start();

        // 清理已关闭的客户端连接
        void ClearClientTimerFunc();

        static ZkClient zkCli; // 静态成员变量，用于管理 Zookeeper 客户端

    private:
        // 初始化服务器，包括创建接收器和事件循环等
        void init();

        // 当有新客户端连接时调用的函数
        void onAccept();

    private:
        TcpAcceptor::TcpAcceptorPtr m_acceptor;               // TCP 连接接收器
        NetAddr::NetAddrPtr m_local_addr;                     // 本地监听地址
        EventLoop *m_main_eventloop{NULL};                    // 主事件循环（mainReactor）
        IOThreadGroup *m_io_thread_groups{NULL};              // IO 线程组（subReactor）
        FdEvent *m_listen_fd_event;                           // 监听文件描述符事件
        int m_client_counts{0};                               // 客户端连接数计数器
        std::set<TcpConnection::TcpConnectionPtr> m_client;   // 客户端连接集合
        TimerEvent::TimerEventPtr m_clear_client_timer_event; // 清理客户端定时器事件
    };

}; // namespace netrpc

#endif // NETRPC_NET_TCP_TCPSERVER_H