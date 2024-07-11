/*
 * @Description: TcpClient 类的实现文件，实现了 TCP 客户端的连接、消息发送与接收等功能。
 * @Version: 1.0
 * @Author: Niu Shuo
 * @Date: 2024-06-13 21:50:24
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 22:14:40
 */
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "netrpc/common/log.h"
#include "netrpc/net/tcp/tcp_client.h"
#include "netrpc/net/eventloop.h"
#include "netrpc/net/fd_event_group.h"
#include "netrpc/common/error_code.h"
#include "netrpc/net/tcp/net_addr.h" 
#include "tcp_client.h"

namespace netrpc
{

    // 构造函数，传入远端地址 peer_addr
    TcpClient::TcpClient(NetAddr::NetAddrPtr peer_addr) : m_peer_addr(peer_addr)
    {
        m_eventloop = EventLoop::GetCurrentEventLoop();
        m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0); // 创建套接字

        if (m_fd < 0)
        {
            ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
            return;
        }

        m_fd_event = FdEventGroup::getFdEventGroup()->getFdEvent(m_fd);
        m_fd_event->setNonBlock(); // 设置非阻塞模式

        m_connection = std::make_shared<TcpConnection>(m_eventloop, m_fd, 128, peer_addr, nullptr, TcpConnectionByClient);
        m_connection->setConnectionType(TcpConnectionByClient); // 设置连接类型
    }

    // 析构函数，释放资源
    TcpClient::~TcpClient()
    {
        DEBUGLOG("TcpClient::~TcpClient()");
        if (m_fd > 0)
        {
            close(m_fd); // 关闭套接字
        }
    }

    // 异步连接远端服务器
    // 如果连接成功，执行 done 函数
    void TcpClient::connect(std::function<void()> done)
    {
        int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
        // rt = 0，连接成功
        if (rt == 0)
        {
            DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
            m_connection->setState(Connected);
            initLocalAddr();
            if (done)
            {
                done();
            }
        }
        else if (rt == -1)
        {
            // rt == -1 && errno == EINPROGRESS 表示连接正在进行中
            if (errno == EINPROGRESS)
            {
                // epoll 监听可写事件，发生可写事件代表连接已经完成或者失败，然后判断错误码
                m_fd_event->listen(FdEvent::OUT_EVENT, [this, done]()
                                   {
                    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
                    // rt < 0 && errno == EISCONN 表示 socket 在之前已经是连接状态
                    if ((rt < 0 && errno == EISCONN) || (rt == 0)) {
                        DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
                        initLocalAddr();
                        m_connection->setState(Connected);
                    } else {
                        if (errno == ECONNREFUSED) { // 连接失败
                            m_connect_error_code = ERROR_PEER_CLOSED;
                            m_connect_error_info = "connect refused, sys error = " + std::string(strerror(errno));
                        } else {
                            m_connect_error_code = ERROR_FAILED_CONNECT;
                            m_connect_error_info = "connect unknown error, sys error =" + std::string(strerror(errno));
                        }
                        ERRORLOG("connect error, errno=%d, error=%s", errno, strerror(errno));
                        close(m_fd);
                        // 没有设置设置未超时重连，所以没有用
                        m_fd = socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
                    }

                    // 连接完后需要去掉可写事件的监听，不然会一直触发
                    m_eventloop->deleteEpollEvent(m_fd_event);
                    DEBUGLOG("now begin to done");
                    // 如果连接完成，才会执行回调函数
                    if (done) {
                        done();
                    } });

                m_eventloop->addEpollEvent(m_fd_event);

                if (!m_eventloop->isLooping())
                {
                    m_eventloop->loop();
                }
            }
            else
            {
                ERRORLOG("connect error, errno=%d, error=%s", errno, strerror(errno));
                m_connect_error_code = ERROR_FAILED_CONNECT;
                m_connect_error_info = "connect error, sys error = " + std::string(strerror(errno));
                if (done)
                {
                    done();
                }
            }
        }
    }

    // 异步发送消息
    void TcpClient::writeMessage(AbstractProtocol::AbstractProtocolPtr message, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done)
    {
        // 1. 把 message 对象写入到 Connection 的 buffer，done 也写入
        // 2. 启动 connection 可写事件
        m_connection->pushSendMessage(message, done);
        m_connection->listenWrite();
    }

    // 异步读取消息
    // 如果读取消息成功，会调用 done 函数，参数为读取的消息对象
    void TcpClient::readMessage(const std::string &msg_id, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done)
    {
        // 1. 监听可读事件
        // 2. 从 buffer 中解码得到消息对象，判断是否 msg_id 相等，相等则读成功，执行其回调
        m_connection->pushReadMessage(msg_id, done);
        m_connection->listenRead();
    }

    // 停止客户端连接
    void TcpClient::stop()
    {
        if (m_eventloop->isLooping())
        {
            m_eventloop->stop();
        }
    }

    // 获取连接的错误码
    int TcpClient::getConnectErrorCode()
    {
        return m_connect_error_code;
    }

    // 获取连接的错误信息
    std::string TcpClient::getConnectErrorInfo()
    {
        return m_connect_error_info;
    }

    // 获取远端地址
    NetAddr::NetAddrPtr TcpClient::getPeerAddr()
    {
        return m_peer_addr;
    }

    // 获取本地地址
    NetAddr::NetAddrPtr TcpClient::getLocalAddr()
    {
        return m_local_addr;
    }

    // 初始化本地地址
    void TcpClient::initLocalAddr()
    {
        sockaddr_in local_addr;
        socklen_t len = sizeof(local_addr);

        int ret = getsockname(m_fd, reinterpret_cast<sockaddr *>(&local_addr), &len);
        if (ret != 0)
        {
            ERRORLOG("initLocalAddr error, getsockname error. errno=%d, error=%s", errno, strerror(errno));
            return;
        }
        m_local_addr = std::make_shared<IPNetAddr>(local_addr);
    }

    // 添加定时器事件
    void TcpClient::addTimerEvent(TimerEvent::TimerEventPtr timer_event)
    {
        m_eventloop->addTimerEvent(timer_event);
    }

    // 获取当前连接
    TcpConnection::TcpConnectionPtr TcpClient::getConnection()
    {
        return m_connection;
    }

}; // namespace netrpc