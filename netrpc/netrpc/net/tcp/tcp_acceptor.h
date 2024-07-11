/*
 * @Description: TcpAcceptor类头文件，用于接受TCP连接
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-07 18:56:30
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 21:17:00
 */
#ifndef NETRPC_NET_TCP_TCPACCEPTOR_H
#define NETRPC_NET_TCP_TCPACCEPTOR_H

#include <memory>
#include "netrpc/net/tcp/net_addr.h"

namespace netrpc
{

    // TcpAcceptor类，用于接受TCP连接
    class TcpAcceptor
    {
    public:
        // TcpAcceptor指针类型
        using TcpAcceptorPtr = std::shared_ptr<TcpAcceptor>;

        // 构造函数，创建TcpAcceptor对象
        TcpAcceptor(NetAddr::NetAddrPtr local_addr);

        // 析构函数，释放资源
        ~TcpAcceptor();

        // 接受客户端连接
        std::pair<int, NetAddr::NetAddrPtr> accept();

        // 获取监听套接字的文件描述符
        int getListenFd();

    private:
        NetAddr::NetAddrPtr m_local_addr; // 服务端监听的地址，addr -> ip:port
        int m_family{-1};                 // 协议簇
        int m_listenfd{-1};               // 监听套接字
    };
};

#endif