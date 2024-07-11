/*
 * @Description: TcpClient 类的声明文件，定义了 TCP 客户端的接口和功能。
 * @Version: 1.0
 * @Author: Niu Shuo
 * @Date: 2024-06-13 21:48:34
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 21:50:01
 */
#ifndef NETRPC_NET_TCP_TCPCLIENT_H
#define NETRPC_NET_TCP_TCPCLIENT_H

#include <memory>
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/eventloop.h"
#include "netrpc/net/tcp/tcp_connection.h"
#include "netrpc/net/coder/abstract_protocol.h"
#include "netrpc/net/timer_event.h"

namespace netrpc
{

    // TcpClient 类，负责管理 TCP 客户端的连接、消息发送与接收等功能
    class TcpClient
    {
    public:
        using TcpClientPtr = std::shared_ptr<TcpClient>; // 定义 TcpClient 智能指针类型

        // 构造函数，传入远端地址 peer_addr
        TcpClient(NetAddr::NetAddrPtr peer_addr);

        // 析构函数，释放资源
        ~TcpClient();

        // 异步连接远端服务器
        // 如果连接成功，执行 done 函数
        void connect(std::function<void()> done);

        // 异步发送消息 message
        // 如果发送成功，执行 done 函数，参数为发送的 message 对象
        void writeMessage(AbstractProtocol::AbstractProtocolPtr message, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done);

        // 异步读取消息，根据 msg_id 查找
        // 如果读取成功，执行 done 函数，参数为读取的 message 对象
        void readMessage(const std::string &msg_id, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done);

        // 停止客户端连接
        void stop();

        // 获取连接的错误码
        int getConnectErrorCode();

        // 获取连接的错误信息
        std::string getConnectErrorInfo();

        // 获取远端地址
        NetAddr::NetAddrPtr getPeerAddr();

        // 获取本地地址
        NetAddr::NetAddrPtr getLocalAddr();

        // 初始化本地地址
        void initLocalAddr();

        // 添加定时器事件
        void addTimerEvent(TimerEvent::TimerEventPtr timer_event);

        // 获取当前连接
        TcpConnection::TcpConnectionPtr getConnection();

    private:
        NetAddr::NetAddrPtr m_peer_addr;              // 远端地址
        NetAddr::NetAddrPtr m_local_addr;             // 本地地址
        EventLoop *m_eventloop{nullptr};              // 事件循环指针
        int m_fd{-1};                                 // 文件描述符
        FdEvent *m_fd_event{nullptr};                 // 文件描述符事件指针
        TcpConnection::TcpConnectionPtr m_connection; // TCP 连接指针
        int m_connect_error_code{0};                  // 连接错误码
        std::string m_connect_error_info;             // 连接错误信息
    };

}; // namespace netrpc

#endif // NETRPC_NET_TCP_TCPCLIENT_H