#ifndef NRPC_NET_SOCKET_H
#define NRPC_NET_SOCKET_H

#include <ostream>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <fcntl.h>

namespace nrpc
{
    // 输出套接字地址信息
    std::ostream &operator<<(std::ostream &os, struct sockaddr_in addr);

    // 根据地址和端口获取套接字地址结构
    struct sockaddr_in get_addr(const char *addr, unsigned short port);

    // 设置套接字为非阻塞模式
    void set_nonblocking(int fd);

    // 创建并监听套接字
    // addr: 地址字符串
    // port: 端口号
    // backlog: 监听队列的最大长度，默认为 1024
    int listen_socket(const char *addr, unsigned short port, int backlog = 1024);
}; // namespace nrpc

#endif