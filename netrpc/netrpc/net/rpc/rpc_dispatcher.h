/*
 * @Description: RpcDispatcher 类负责管理和调度 RPC 服务。
 *                它实现了服务注册、请求调度和错误处理等功能。
 * @Version: 1.0
 * @Author: Niu Shuo
 * @Date: 2024-06-13 20:55:36
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-07-03 20:50:25
 */
#ifndef NETRPC_NET_RPC_RPC_DISPATCHER_H
#define NETRPC_NET_RPC_RPC_DISPATCHER_H

#include <map>
#include <memory>
#include <google/protobuf/service.h>
#include "netrpc/net/coder/abstract_protocol.h"
#include "netrpc/net/coder/tinypb_protocol.h"

namespace netrpc {

    class TcpConnection; // 前置声明 TcpConnection 类

    class RpcDispatcher {
    public:
        // 禁用拷贝构造和赋值运算符，确保单例模式下只有一个实例
        RpcDispatcher(const RpcDispatcher&) = delete;
        RpcDispatcher& operator=(const RpcDispatcher&) = delete;

        // 获取单例实例的静态方法
        static RpcDispatcher& GetInst() {
            static RpcDispatcher inst;
            return inst;
        }

    public:
        using ServicePtr = std::shared_ptr<google::protobuf::Service>; // 定义服务对象的智能指针类型

        // 分发 RPC 请求的方法，根据请求调用相应的服务方法，并处理响应
        void dispatch(AbstractProtocol::AbstractProtocolPtr request, AbstractProtocol::AbstractProtocolPtr response, TcpConnection* connection);

        // 注册 RPC 服务的方法，将服务对象注册到服务映射中
        void registerService(ServicePtr service);

        // 设置 TinyPB 协议的错误信息，用于处理解析或调用过程中的错误
        void setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string err_info);
    private:
        bool parseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name);
    private:
        // 私有构造函数，确保只能通过 GetInst 方法获取实例
        RpcDispatcher() = default;

        // 内部结构体，用于保存每个服务对象及其方法映射
        struct ServiceInfo {
            ServicePtr m_service; // 保存服务对象
            std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap; // 保存服务方法的映射
        };

        std::map<std::string, ServiceInfo> m_service_map; // 保存所有注册的服务对象的映射表
    };

}; // namespace netrpc

#endif // NETRPC_NET_RPC_RPC_DISPATCHER_H