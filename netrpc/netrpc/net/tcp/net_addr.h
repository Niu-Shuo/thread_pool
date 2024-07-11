/*
 * @Description: net_addr.h 网络地址类声明
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-03 17:38:17
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 21:15:20
 */
#ifndef NETRPC_NET_TCP_NETADDR_H
#define NETRPC_NET_TCP_NETADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>

namespace netrpc
{
    // 网络地址类，定义了获取套接字地址、套接字长度、地址族、地址字符串和有效性检查等接口
    class NetAddr
    {
    public:
        using NetAddrPtr = std::shared_ptr<NetAddr>;
        
        // 获取套接字地址
        virtual sockaddr* getSockAddr() = 0;
        
        // 获取套接字长度
        virtual socklen_t getSockLen() = 0;
        
        // 获取地址族
        virtual int getFamily() = 0;
        
        // 获取地址字符串表示
        virtual std::string toString() = 0;
        
        // 检查地址是否有效
        virtual bool checkValid() = 0;
    };

    // IP地址类，继承自NetAddr，实现了具体的地址操作
    class IPNetAddr : public NetAddr {
    public:
        // 检查IP地址是否有效的静态成员函数
        static bool CheckValid(const std::string& addr);

    public:
        // 构造函数，接受IP地址和端口号
        IPNetAddr(const std::string& ip, uint16_t port);

        // 构造函数，接受完整的IP地址
        IPNetAddr(const std::string& addr);

        // 构造函数，接受sockaddr_in结构
        IPNetAddr(sockaddr_in addr);

        // 获取套接字地址
        sockaddr* getSockAddr() override;

        // 获取套接字长度
        socklen_t getSockLen() override;

        // 获取地址族
        int getFamily() override;

        // 获取地址字符串表示
        std::string toString() override;

        // 检查地址是否有效
        bool checkValid() override;

    private:
        std::string m_ip;     // IP地址
        uint16_t m_port {0};  // 端口号
        sockaddr_in m_addr;   // 套接字地址结构
    };

};

#endif