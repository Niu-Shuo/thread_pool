/*
 * @Description: RpcController 类的声明文件，定义了用于控制 RPC 调用的接口。
 * @Version: 1.0
 * @Author: Niu Shuo
 * @Date: 2024-06-13 22:15:32
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 22:22:27
 */
#ifndef NETRPC_NET_RPC_RPC_CONTROLLER_H
#define NETRPC_NET_RPC_RPC_CONTROLLER_H

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <string>
#include "netrpc/net/tcp/net_addr.h"
#include "netrpc/common/log.h"

namespace netrpc
{

    // RpcController 类继承自 protobuf 的 RpcController
    class RpcController : public google::protobuf::RpcController
    {
    public:
        RpcController() { INFOLOG("RpcController"); }   // 构造函数，记录日志
        ~RpcController() { INFOLOG("~RpcController"); } // 析构函数，记录日志

        // 重置 RPC 控制器状态
        void Reset();

        // 检查是否发生错误
        bool Failed() const;

        // 获取错误文本
        std::string ErrorText() const;

        // 开始取消 RPC
        void StartCancel();

        // 设置失败原因
        void SetFailed(const std::string &reason);

        // 检查是否取消
        bool IsCanceled() const;

        // 通知取消事件
        void NotifyOnCancel(google::protobuf::Closure *callback);

        // 设置错误信息
        void SetError(int32_t error_code, const std::string error_info);

        // 获取错误码
        int32_t GetErrorCode();

        // 获取错误信息
        std::string GetErrorInfo();

        // 设置消息 ID
        void SetMsgId(const std::string &msg_id);

        // 获取消息 ID
        std::string GetMsgId();

        // 设置本地地址
        void SetLocalAddr(NetAddr::NetAddrPtr addr);

        // 设置远端地址
        void SetPeerAddr(NetAddr::NetAddrPtr addr);

        // 获取本地地址
        NetAddr::NetAddrPtr GetLocalAddr();

        // 获取远端地址
        NetAddr::NetAddrPtr GetPeerAddr();

        // 设置超时时间
        void SetTimeout(int timeout);

        // 获取超时时间
        int GetTimeout();

        // 检查是否完成
        bool Finished();

        // 设置完成状态
        void SetFinished(bool value);

        // 获取最大重试次数
        int GetMaxRetry();

        // 设置最大重试次数
        void SetMaxRetry(const int maxRetry);

    private:
        int32_t m_error_code{0};          // 错误码
        std::string m_error_info;         // 错误信息
        std::string m_msg_id;             // 消息 ID
        bool m_is_failed{false};          // 是否失败
        bool m_is_canceled{false};        // 是否取消
        bool m_is_finished{false};        // 是否完成
        NetAddr::NetAddrPtr m_local_addr; // 本地地址
        NetAddr::NetAddrPtr m_peer_addr;  // 远端地址
        int m_timeout{1000};              // 超时时间，单位：毫秒
        int m_max_retry{2};               // 最大重试次数
    };
};

#endif