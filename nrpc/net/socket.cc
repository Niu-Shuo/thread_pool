#include "socket.h"
#include "base/logging.h"

namespace nrpc {

// 重载输出运算符，将 sockaddr_in 结构输出为 IP 地址和端口号的形式
std::ostream &nrpc::operator<<(std::ostream &os, sockaddr_in addr)
{
    char buf[INET_ADDRSTRLEN] = {"\0"};
    inet_ntop(AF_INET, &addr.sin_addr, buf, INET_ADDRSTRLEN);
    os << buf << ":" << ntohs(addr.sin_port);
    return os;
}

// 根据地址字符串和端口号获取 sockaddr_in 结构
sockaddr_in get_addr(const char *addr, unsigned short port)
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = ::htons(port);
    int err = ::inet_pton(AF_INET, addr, &server_addr.sin_addr);
    PLOG_FATAL_IF(err != 1) << "failed to invoke ::inet_pton";
    return server_addr;
}

// 设置套接字为非阻塞模式
void set_nonblocking(int fd)
{
    int flags = ::fcntl(fd, F_GETFL);
    int err = ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    PLOG_FATAL_IF(err == -1) << "failed to invoke ::fcntl";
}

// 创建并监听套接字
int listen_socket(const char *addr, unsigned short port, int backlog)
{
    // 创建套接字并设置为非阻塞模式
    int listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    PLOG_FATAL_IF(listen_fd == -1) << "failed to invoke ::socket";
    set_nonblocking(listen_fd);

    // 设置套接字选项 SO_REUSEADDR
    int option = 1;
    int err = ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&option, sizeof(option));
    PLOG_FATAL_IF(err == -1) << "failed to invoke ::setsockopt SO_REUSEADDR";

    // 绑定地址和端口
    struct sockaddr_in server_addr = get_addr(addr, port);
    err = ::bind(listen_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    PLOG_FATAL_IF(err == -1) << "failed to invoke ::bind. addr: " << server_addr;

    // 监听连接
    err = ::listen(listen_fd, backlog);
    PLOG_FATAL_IF(err == -1) << "failed to invoke ::listen. addr: " << server_addr;

    return listen_fd;
}

};  // namespace nrpc