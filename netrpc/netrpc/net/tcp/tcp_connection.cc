/*
 * @Description: TCP 连接类的实现，包含读写事件处理、连接状态管理等功能
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-13 20:57:13
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-14 14:52:56
 */
#include <unistd.h>
#include "netrpc/common/log.h"
#include "netrpc/net/fd_event_group.h"
#include "netrpc/net/tcp/tcp_connection.h"
#include "netrpc/net/coder/string_coder.h"
#include "netrpc/net/coder/tinypb_coder.h"

namespace netrpc {

    // 构造函数，初始化 TCP 连接对象
    TcpConnection::TcpConnection(EventLoop *eventloop, int fd, int buffer_size, NetAddr::NetAddrPtr peer_addr, NetAddr::NetAddrPtr local_addr, TcpConnectionType type /* = TcpConnectionByServer*/)
        : m_eventloop(eventloop), m_local_addr(local_addr), m_peer_addr(peer_addr), m_state(NotConnected), m_fd(fd), m_connection_type(type)
    {
        // 初始化输入输出缓冲区
        m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

        // 获取文件描述符事件并设置为非阻塞模式
        m_fd_event = FdEventGroup::getFdEventGroup()->getFdEvent(fd);
        m_fd_event->setNonBlock();

        // 初始化编码解码器
        m_coder = new TinyPBCoder();

        // 如果是作为服务器使用，则监听读事件
        if (m_connection_type == TcpConnectionByServer) {
            listenRead();
        }
    }

    // 析构函数，清理资源
    TcpConnection::~TcpConnection()
    {
        DEBUGLOG("~TcpConnection");
        if (m_coder) {
            delete m_coder;
            m_coder = nullptr;
        }
    }

    // 读事件处理函数
    void TcpConnection::onRead()
    {
        // 检查连接状态
        if (m_state != Connected) {
            ERRORLOG("onRead error, client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
            return;
        }

        bool is_read_all = false;
        bool is_close = false;

        // 循环读取数据
        while (!is_read_all) {
            // 如果缓冲区满了，扩展缓冲区
            if (m_in_buffer->writeAble() == 0) {
                m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
            }

            int read_count = m_in_buffer->writeAble();
            int write_index = m_in_buffer->writeIndex();
            int rt = read(m_fd, &(m_in_buffer->m_buffer[write_index]), read_count);

            // 记录读取的字节数
            DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", rt, m_peer_addr->toString().c_str(), m_fd);

            if (rt > 0) {
                m_in_buffer->moveWriteIndex(rt);
                if (rt == read_count) {
                    continue;
                } else if (rt < read_count) {
                    is_read_all = true;
                    break;
                }
            } else if (rt == 0) {
                is_close = true;
                break;
            } else if (rt == -1 && errno == EAGAIN) {
                is_read_all = true;
                break;
            }
        }

        // 处理连接关闭情况
        if (is_close) {
            INFOLOG("peer closed, peer addr [%d], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
            clear();
            return;
        }

        // 处理未读完的数据
        if (!is_read_all) {
            ERRORLOG("not read all data");
        }

        // 执行业务逻辑
        excute();
    }

    // 执行业务逻辑函数
    void TcpConnection::excute()
    {
        if (m_connection_type == TcpConnectionByServer) {
            std::vector<AbstractProtocol::AbstractProtocolPtr> result;
            std::vector<AbstractProtocol::AbstractProtocolPtr> replay_messages;
            m_coder->decode(result, m_in_buffer);

            for (size_t ii = 0; ii < result.size(); ++ii) {
                INFOLOG("sucsess get request[%s] from client[%s]", result[ii]->m_msg_id.c_str(), m_peer_addr->toString().c_str());

                auto message = std::make_shared<TinyPBProtocol>();
                RpcDispatcher::GetInst().dispatch(result[ii], message, this);
                replay_messages.emplace_back(message);
            }

            m_coder->encode(replay_messages, m_out_buffer);
            listenWrite();
        } else {
            std::vector<AbstractProtocol::AbstractProtocolPtr> result;
            m_coder->decode(result, m_in_buffer);

            for (size_t ii = 0; ii < result.size(); ++ii) {
                std::string msg_id = result[ii]->m_msg_id;
                auto it = m_read_dones.find(msg_id);
                if (it != m_read_dones.end()) {
                    it->second(result[ii]);
                    m_read_dones.erase(it);
                }
            }
        }
    }

    // 写事件处理函数
    void TcpConnection::onWrite()
    {
        // 检查连接状态
        if (m_state != Connected) {
            ERRORLOG("onWrite error, client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
            return;
        }

        // 如果是客户端，则将消息编码后放入输出缓冲区
        if (m_connection_type == TcpConnectionByClient) {
            std::vector<AbstractProtocol::AbstractProtocolPtr> messages;
            for (size_t ii = 0; ii < m_write_dones.size(); ++ii) {
                messages.push_back(m_write_dones[ii].first);
            }

            m_coder->encode(messages, m_out_buffer);
        }

        bool is_write_all = false;
        while (true) {
            if (m_out_buffer->readAble() == 0) {
                DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
                is_write_all = true;
                break;
            }

            int write_size = m_out_buffer->readAble();
            int read_index = m_out_buffer->readIndex();

            int rt = write(m_fd, &(m_out_buffer->m_buffer[read_index]), write_size);
            if (rt >= write_size) {
                DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
                is_write_all = true;
                break;
            } else if (rt == -1 && errno == EAGAIN) {
                ERRORLOG("write data error, errno==EAGAIN and rt == -1");
                break;
            }
        }

        // 取消监听写事件
        if (is_write_all) {
            m_fd_event->cancle(FdEvent::OUT_EVENT);
            m_eventloop->addEpollEvent(m_fd_event);
        }

        // 如果是客户端，执行写完成的回调
        if (m_connection_type == TcpConnectionByClient) {
            for (size_t ii = 0; ii < m_write_dones.size(); ++ii) {
                m_write_dones[ii].second(m_write_dones[ii].first);
            }
            m_write_dones.clear();
        }
    }

    // 设置连接状态
    void TcpConnection::setState(const TcpState state)
    {
        m_state = state;
    }

    // 获取连接状态
    TcpState TcpConnection::getState()
    {
        return m_state;
    }

    // 清理连接资源
    void TcpConnection::clear()
    {
        if (m_state == Closed) {
            return;
        }
        m_fd_event->cancle(FdEvent::IN_EVENT);
        m_fd_event->cancle(FdEvent::OUT_EVENT);
        m_eventloop->deleteEpollEvent(m_fd_event);

        m_state = Closed;
    }

    // 获取文件描述符
    int TcpConnection::getFd()
    {
        return m_fd;
    }

    // 关闭连接
    void TcpConnection::shutdown()
    {
        if (m_state == Closed || m_state == NotConnected) {
            return;
        }

        m_state = HalfClosing;

        // 调用 shutdown 关闭读写，触发四次挥手的第一个阶段
        ::shutdown(m_fd, SHUT_RDWR);
    }

    // 设置连接类型
    void TcpConnection::setConnectionType(TcpConnectionType type)
    {
        m_connection_type = type;
    }

    // 监听写事件
    void TcpConnection::listenWrite()
    {
        m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
        m_eventloop->addEpollEvent(m_fd_event);
    }

    // 监听读事件
    void TcpConnection::listenRead()
    {
        m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
        m_eventloop->addEpollEvent(m_fd_event);
    }

    // 推送发送消息
    void TcpConnection::pushSendMessage(AbstractProtocol::AbstractProtocolPtr message, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done)
    {
        m_write_dones.push_back(std::make_pair(message, done));
    }

    // 推送读取消息
    void TcpConnection::pushReadMessage(const std::string &msg_id, std::function<void(AbstractProtocol::AbstractProtocolPtr)> done)
    {
        m_read_dones.insert(std::make_pair(msg_id, done));
    }

    // 获取本地地址
    NetAddr::NetAddrPtr TcpConnection::getLocalAddr()
    {
        return m_local_addr;
    }

    // 获取对端地址
    NetAddr::NetAddrPtr TcpConnection::getPeerAddr()
    {
        return m_peer_addr;
    }

    // 回复消息
    void TcpConnection::reply(std::vector<AbstractProtocol::AbstractProtocolPtr> &replay_messages)
    {
        m_coder->encode(replay_messages, m_out_buffer);
        listenWrite();
    }
};