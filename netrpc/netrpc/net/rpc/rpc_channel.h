/*
 * @Description: RpcChannel类的实现，用于RPC调用的通道
 * @Version: 1.0
 * @Author: Niu Shuo
 * @Date: 2024-06-13 22:30:59
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 22:40:50
 */
#ifndef NETRPC_NET_RPC_RPC_CHANNEL_H
#define NETRPC_NET_RPC_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include <memory>
#include <vector>
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/common/zookeeperutil.h"
#include "netrpc/net/tcp/tcp_client.h"
#include "netrpc/net/timer_event.h"
#include "netrpc/net/load_balance.h"

namespace netrpc
{
// 宏定义，创建一个新的消息对象
#define NEWMESSAGE(type, var_name) \
    std::shared_ptr<type> var_name = std::make_shared<type>();

// 宏定义，创建一个新的RpcController对象
#define NEWRPCCONTROLLER(var_name) \
    std::shared_ptr<netrpc::RpcController> var_name = std::make_shared<netrpc::RpcController>();

// 宏定义，创建一个新的RpcChannel对象
#define NEWRPCCHANNEL(addr, var_name) \
    std::shared_ptr<netrpc::RpcChannel> var_name = std::make_shared<netrpc::RpcChannel>(netrpc::RpcChannel::FindAddr(addr));

// 宏定义，用于调用RPC方法
#define CALLRPC(addr, stub_name, method_name, controller, request, response, closure)                         \
    {                                                                                                         \
        NEWRPCCHANNEL(addr, channel);                                                                         \
        channel->Init(controller, request, response, closure);                                                \
        stub_name(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
    }

    // RpcChannel类，继承自google::protobuf::RpcChannel，并启用共享指针管理
    class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel>
    {
    public:
        using RpcChannelPtr = std::shared_ptr<RpcChannel>;
        using ControllerPtr = std::shared_ptr<google::protobuf::RpcController>;
        using MessagePtr = std::shared_ptr<google::protobuf::Message>;
        using ClosurePtr = std::shared_ptr<google::protobuf::Closure>;

    public:
        // 静态方法，获取地址
        // 若传入的字符串是ip:port，直接返回对应的地址
        // 否则认为是rpc服务名，从配置文件中获取对应的ip:port
        static NetAddr::NetAddrPtr FindAddr(const std::string &str);

    public:
        // 构造函数，接受一个目标地址
        RpcChannel(NetAddr::NetAddrPtr peer_addr);

        // 构造函数，接受一个地址向量和负载均衡策略
        RpcChannel(std::vector<NetAddr::NetAddrPtr> addrs, LoadBalanceCategory loadBalance = LoadBalanceCategory::Random);

        // 析构函数
        ~RpcChannel();

        // 重载的CallMethod方法，用于调用RPC方法
        void CallMethod(const google::protobuf::MethodDescriptor *method,
                        google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                        google::protobuf::Message *response, google::protobuf::Closure *done);

        // 初始化RpcChannel
        void Init(ControllerPtr controller, MessagePtr req, MessagePtr res, ClosurePtr done);

        // 获取当前的RpcController
        google::protobuf::RpcController *getController();

        // 获取当前的请求消息
        google::protobuf::Message *getRequest();

        // 获取当前的响应消息
        google::protobuf::Message *getResponse();

        // 获取当前的Closure对象
        google::protobuf::Closure *getClosure();

        // 获取当前的TcpClient
        TcpClient *getTcpClient();

    private:
        // 回调函数，用于处理RPC调用完成后的操作
        void callBack();

    private:
        // 成员变量，存储目标地址和本地地址
        NetAddr::NetAddrPtr m_peer_addr{nullptr};
        NetAddr::NetAddrPtr m_local_addr{nullptr};

        // 成员变量，存储控制器、请求消息、响应消息和Closure对象
        ControllerPtr m_controller{nullptr};
        MessagePtr m_request{nullptr};
        MessagePtr m_response{nullptr};
        ClosurePtr m_closure{nullptr};

        // 标志变量，表示是否初始化
        bool m_is_init{false};

        // 成员变量，存储TcpClient和地址向量
        TcpClient::TcpClientPtr m_client{nullptr};
        std::vector<NetAddr::NetAddrPtr> m_addrs;

        // 成员变量，存储负载均衡策略
        LoadBalanceStrategy::ptr m_loadBalancer;
    };
};

#endif