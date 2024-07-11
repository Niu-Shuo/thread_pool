/*
 * @Description: 定义了抽象协议类 AbstractProtocol，包含了消息ID作为唯一标识请求或响应的功能。
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-07 21:12:31
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 19:50:16
 */

#ifndef NETRPC_NET_CODER_ABSTRACT_PROTOCOL_H
#define NETRPC_NET_CODER_ABSTRACT_PROTOCOL_H

#include <memory> // 引入智能指针
#include <string> // 引入字符串类

namespace netrpc
{

    // 定义抽象协议类 AbstractProtocol，继承自 std::enable_shared_from_this，支持智能指针
    struct AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol>
    {
    public:
        using AbstractProtocolPtr = std::shared_ptr<AbstractProtocol>; // 定义抽象协议指针类型
        virtual ~AbstractProtocol() {}                                 // 虚析构函数，支持多态

    public:
        std::string m_msg_id; // 消息ID，用于唯一标识一个请求或响应
    };

}; // namespace netrpc

#endif // NETRPC_NET_CODER_ABSTRACT_PROTOCOL_H