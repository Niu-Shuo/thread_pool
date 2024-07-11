/*
 * @Description: TcpAcceptor类实现文件，用于接受TCP连接
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-07 19:19:36
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-07 20:17:22
 */
#include <assert.h>
#include <unistd.h>
#include "netrpc/net/tcp/tcp_acceptor.h"
#include "netrpc/common/log.h"

namespace netrpc
{

    // 构造函数，根据给定的本地地址创建TcpAcceptor对象
    TcpAcceptor::TcpAcceptor(NetAddr::NetAddrPtr local_addr) : m_local_addr(local_addr)
    {
        // 检查本地地址是否有效
        if (!local_addr->checkValid())
        {
            ERRORLOG("Invalid local address: %s", local_addr->toString().c_str());
            exit(EXIT_FAILURE);
        }

        // 获取协议簇和创建监听套接字
        m_family = m_local_addr->getFamily();
        m_listenfd = socket(m_family, SOCK_STREAM, 0);
        assert(m_listenfd >= 0);

        // 设置套接字选项，允许地址重用
        int val = 1;
        if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0)
        {
            ERRORLOG("setsockopt SO_REUSEADDR error: %s", strerror(errno));
        }

        // 绑定地址到套接字
        socklen_t len = m_local_addr->getSockLen();
        if (bind(m_listenfd, m_local_addr->getSockAddr(), len) != 0)
        {
            ERRORLOG("bind error: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        // 开始监听连接请求
        if (listen(m_listenfd, 1000) != 0)
        {
            ERRORLOG("listen error: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // 析构函数，释放资源
    TcpAcceptor::~TcpAcceptor()
    {
        // 关闭监听套接字
        if (m_listenfd >= 0)
        {
            close(m_listenfd);
        }
    }

    // 接受客户端连接，返回连接的文件描述符和客户端地址信息
    std::pair<int, NetAddr::NetAddrPtr> TcpAcceptor::accept()
    {
        if (m_family == AF_INET)
        {
            sockaddr_in client_addr;
            memset(&client_addr, 0, sizeof(client_addr));
            socklen_t client_addr_len = sizeof(client_addr);

            // 接受连接
            int client_fd = ::accept(m_listenfd, reinterpret_cast<sockaddr *>(&client_addr), &client_addr_len);
            if (client_fd < 0)
            {
                ERRORLOG("accept error: %s", strerror(errno));
            }

            // 获取客户端地址信息
            IPNetAddr::NetAddrPtr peer_addr = std::make_shared<IPNetAddr>(client_addr);
            INFOLOG("A client has been accepted successfully, peer address: %s", peer_addr->toString().c_str());

            return std::make_pair(client_fd, peer_addr);
        }
        else
        {
            // 如果协议簇不是IPv4，可以根据需要实现对应的逻辑
            // 例如，处理IPv6连接，或者抛出异常等
            // 这里暂时返回无效文件描述符和空地址指针
            return std::make_pair(-1, nullptr);
        }
    }

    // 获取监听套接字的文件描述符
    int TcpAcceptor::getListenFd()
    {
        return m_listenfd;
    }

};