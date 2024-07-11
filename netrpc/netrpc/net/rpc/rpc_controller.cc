/*
 * @Description: RPC控制器类的实现，管理RPC调用的状态和信息
 * @Version: 1.0
 * @Author: Niu Shuo
 * @Date: 2024-06-13 22:17:56
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 22:32:03
 */
#include "netrpc/net/rpc/rpc_controller.h"
#include "rpc_controller.h"

namespace netrpc {

    // 重置所有成员变量，恢复初始状态
    void RpcController::Reset() {
        m_error_code = 0;
        m_error_info = "";
        m_msg_id = "";
        m_is_failed = false;
        m_is_canceled = false;
        m_is_finished = false;
        m_local_addr = nullptr;
        m_peer_addr = nullptr;
        m_timeout = 1000;   // 默认超时时间为1000毫秒
    }

    // 检查RPC调用是否失败
    bool RpcController::Failed() const {
        return m_is_failed;
    }

    // 返回错误信息文本
    std::string RpcController::ErrorText() const {
        return m_error_info;
    }

    // 开始取消RPC调用，设置取消状态和失败状态
    void RpcController::StartCancel() {
        m_is_canceled = true;
        m_is_failed = true;
        SetFinished(true);
    }

    // 设置失败状态和错误信息
    void RpcController::SetFailed(const std::string &reason) {
        m_error_info = reason;
        m_is_failed = true;
    }

    // 检查RPC调用是否被取消
    bool RpcController::IsCanceled() const {
        return m_is_canceled;
    }

    // 注册一个回调函数，当调用被取消时调用该回调
    void RpcController::NotifyOnCancel(google::protobuf::Closure *callback) {
        if (IsCanceled() && callback) {
            callback->Run();
        }
    }

    // 设置错误码和错误信息
    void RpcController::SetError(int32_t error_code, const std::string error_info) {
        m_error_code = error_code;
        m_error_info = error_info;
        m_is_failed = true;
    }

    // 返回错误码
    int32_t RpcController::GetErrorCode() {
        return m_error_code;
    }

    // 返回错误信息
    std::string RpcController::GetErrorInfo() {
        return m_error_info;
    }

    // 设置消息ID
    void RpcController::SetMsgId(const std::string &msg_id) {
        m_msg_id = msg_id;
    }

    // 返回消息ID
    std::string RpcController::GetMsgId() {
        return m_msg_id;
    }

    // 设置本地地址
    void RpcController::SetLocalAddr(NetAddr::NetAddrPtr addr) {
        m_local_addr = addr;
    }

    // 设置对等地址
    void RpcController::SetPeerAddr(NetAddr::NetAddrPtr addr) {
        m_peer_addr = addr;
    }

    // 返回本地地址
    NetAddr::NetAddrPtr RpcController::GetLocalAddr() {
        return m_local_addr;
    }

    // 返回对等地址
    NetAddr::NetAddrPtr RpcController::GetPeerAddr() {
        return m_peer_addr;
    }

    // 设置超时时间
    void RpcController::SetTimeout(int timeout) {
        m_timeout = timeout;
    }

    // 返回超时时间
    int RpcController::GetTimeout() {
        return m_timeout;
    }

    // 检查RPC调用是否完成
    bool RpcController::Finished() {
        return m_is_finished;
    }

    // 设置RPC调用完成状态
    void RpcController::SetFinished(bool value) {
        m_is_finished = value;
    }

    // 返回最大重试次数
    int RpcController::GetMaxRetry() {
        return m_max_retry;
    }

    // 设置最大重试次数
    void RpcController::SetMaxRetry(const int maxRetry) {
        m_max_retry = maxRetry;
    }
};