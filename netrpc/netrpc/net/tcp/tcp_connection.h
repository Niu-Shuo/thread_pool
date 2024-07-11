/*
 * @Description: TCP 连接类的定义，包含读写事件处理、连接状态管理等功能
 * @Version: 1.0
 * @Author: Niu Shuo
 * @Date: 2024-06-13 20:57:08
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-14 12:23:14
 */
#ifndef NETRPC_NET_TCP_TCPCONNECTION_H
#define NETRPC_NET_TCP_TCPCONNECTION_H

#include <memory>
#include <map>
#include <queue>
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/net/tcp/tcp_buffer.h"
#include "netrpc/net/io_thread.h"
#include "netrpc/net/coder/abstract_coder.h"
#include "netrpc/net/rpc/rpc_dispatcher.h"

namespace netrpc
{

    // 定义TCP连接的状态
    enum TcpState
    {
        NotConnected = 1, // 未连接
        Connected = 2,    // 已连接
        HalfClosing = 3,  // 半关闭
        Closed = 4,       // 已关闭
    };

    // 定义TCP连接的类型
    enum TcpConnectionType
    {
        TcpConnectionByServer = 1, // 作为服务端使用，代表跟对端客户端的连接
        TcpConnectionByClient = 2, // 作为客户端使用，代表跟对端服务端的连接
    };

    class TcpConnection
    {
    public:
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>; // 定义智能指针类型
    public:
        // 构造函数，初始化TCP连接
        TcpConnection(EventLoop *eventloop, int fd, int buffer_size, NetAddr::NetAddrPtr peer_addr, NetAddr::NetAddrPtr local_addr, TcpConnectionType type = TcpConnectionByServer);

        // 析构函数，清理资源
        ~TcpConnection();

        // 读事件处理
        void onRead();

        // 执行函数
        void excute();

        // 写事件处理
        void onWrite();

        // 设置连接状态
        void setState(const TcpState state);

        // 获取连接状态
        TcpState getState();

        // 清理连接
        void clear();

        // 获取文件描述符
        int getFd();

        // 服务器主动关闭连接
        void shutdown();

        // 设置连接类型
        void setConnectionType(TcpConnectionType type);

        // 启动监听可写事件
        void listenWrite();

        // 启动监听可读事件
        void listenRead();

        // 添加发送消息及其完成回调
        void pushSendMessage(AbstractProtocol::AbstractProtocolPtr message, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done);

        // 添加读取消息及其完成回调
        void pushReadMessage(const std::string &msg_id, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done);

        // 获取本地地址
        NetAddr::NetAddrPtr getLocalAddr();

        // 获取对端地址
        NetAddr::NetAddrPtr getPeerAddr();

        // 回复消息
        void reply(std::vector<AbstractProtocol::AbstractProtocolPtr> &replay_messages);

    private:
        EventLoop *m_eventloop{NULL};                               // 代表持有该连接的 IO 线程
        NetAddr::NetAddrPtr m_local_addr;                           // 本地地址
        NetAddr::NetAddrPtr m_peer_addr;                            // 对端地址
        TcpBuffer::TcpBufferPtr m_in_buffer;                        // 输入缓冲区
        TcpBuffer::TcpBufferPtr m_out_buffer;                       // 输出缓冲区
        FdEvent *m_fd_event{NULL};                                  // 文件描述符事件
        AbstractCoder *m_coder{NULL};                               // 编码解码器
        TcpState m_state;                                           // 连接状态
        int m_fd{0};                                                // 文件描述符
        TcpConnectionType m_connection_type{TcpConnectionByServer}; // 连接类型
        // 客户端发送完消息执行的回调
        std::vector<std::pair<AbstractProtocol::AbstractProtocolPtr, std::function<void(AbstractProtocol::AbstractProtocolPtr)>>> m_write_dones;
        // 客户端收到消息执行的回调
        // key 为 msg_id
        std::map<std::string, std::function<void(AbstractProtocol::AbstractProtocolPtr)>> m_read_dones;
    };
};

#endif