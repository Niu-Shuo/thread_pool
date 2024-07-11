/*
 * @Description: net_addr.cc 网络地址类实现
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 18:02:37
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-03 19:31:41
 */
#include <string.h>                  // 引入用于操作C风格字符串的库
#include "netrpc/common/log.h"       // 引入日志库
#include "netrpc/net/tcp/net_addr.h" // 引入网络地址类的声明

namespace netrpc
{
    // 检查IP地址和端口是否有效
    bool IPNetAddr::CheckValid(const std::string &addr)
    {
        size_t i = addr.find_first_of(":"); // 查找冒号的位置
        if (i == addr.npos)                 // 如果找不到冒号，地址无效
        {
            return false;
        }
        std::string ip = addr.substr(0, i);                         // 提取IP部分
        std::string port = addr.substr(i + 1, addr.size() - i - 1); // 提取端口部分
        if (ip.empty() || port.empty())                             // IP或端口为空，无效
        {
            return false;
        }
        int iport = std::stoi(port.c_str()); // 将端口转换为整数
        if (iport <= 0 || iport > 65536)     // 检查端口范围
        {
            return false;
        }
        return true;
    }

    // 使用IP和端口构造IPNetAddr对象
    IPNetAddr::IPNetAddr(const std::string &ip, uint16_t port)
    {
        memset(&m_addr, 0, sizeof(m_addr));             // 清零地址结构体
        m_addr.sin_family = AF_INET;                    // 设置地址族为IPv4
        m_addr.sin_addr.s_addr = inet_addr(ip.c_str()); // 设置IP地址
        m_addr.sin_port = htons(port);                  // 设置端口并转换为网络字节序
    }

    // 使用"IP:端口"形式的地址字符串构造IPNetAddr对象
    IPNetAddr::IPNetAddr(const std::string &addr)
    {
        size_t i = addr.find_first_of(":"); // 查找冒号位置
        if (i == addr.npos)                 // 如果找不到冒号，地址无效
        {
            ERRORLOG("invalid ipv4 addr %s", addr.c_str()); // 记录错误日志
            return;
        }
        m_ip = addr.substr(0, i);                                            // 提取IP部分
        m_port = std::atoi(addr.substr(i + 1, addr.size() - i - 1).c_str()); // 提取并转换端口

        memset(&m_addr, 0, sizeof(m_addr));               // 清零地址结构体
        m_addr.sin_family = AF_INET;                      // 设置地址族为IPv4
        m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str()); // 设置IP地址
        m_addr.sin_port = htons(m_port);                  // 设置端口并转换为网络字节序
    }

    // 使用sockaddr_in结构体构造IPNetAddr对象
    IPNetAddr::IPNetAddr(sockaddr_in addr) : m_addr(addr)
    {
        m_ip = std::string(inet_ntoa(m_addr.sin_addr)); // 提取IP地址
        m_port = ntohs(m_addr.sin_port);                // 提取端口并转换为主机字节序
    }

    // 获取sockaddr结构体指针
    sockaddr *IPNetAddr::getSockAddr()
    {
        return reinterpret_cast<sockaddr *>(&m_addr);
    }

    // 获取sockaddr结构体的大小
    socklen_t IPNetAddr::getSockLen()
    {
        return sizeof(m_addr);
    }

    // 获取地址族
    int IPNetAddr::getFamily()
    {
        return AF_INET;
    }

    // 返回"IP:端口"形式的字符串
    std::string IPNetAddr::toString()
    {
        std::string result;
        result = m_ip + ":" + std::to_string(m_port);
        return result;
    }

    // 检查地址的有效性
    bool IPNetAddr::checkValid()
    {
        if (m_ip.empty()) // IP为空，无效
        {
            return false;
        }
        if (m_port <= 0 || m_port > 65536) // 端口范围无效
        {
            return false;
        }
        if (inet_addr(m_ip.c_str()) == INADDR_NONE) // IP格式无效
        {
            return false;
        }
        return true;
    }
}