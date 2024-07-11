/*
 * @Description: RpcClosure类的实现，用于封装回调函数
 * @Version: 1.0
 * @Author: Niu Shuo
 * @Date: 2024-06-13 22:16:15
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-07-03 21:19:16
 */
#ifndef NETRPC_NET_RPC_RPC_CLOSURE_H
#define NETRPC_NET_RPC_RPC_CLOSURE_H

#include <google/protobuf/stubs/callback.h>
#include <functional>
#include <memory>
#include "netrpc/common/run_time.h"
#include "netrpc/common/log.h"
#include "netrpc/common/exception.h"

namespace netrpc {

    // RpcClosure类继承自google::protobuf::Closure，用于封装回调函数
    class RpcClosure : public google::protobuf::Closure {
    public:
        // 构造函数，接受一个std::function<void()>类型的回调函数，并进行初始化
        RpcClosure(std::function<void()> cb) : m_cb(cb) {
            INFOLOG("RpcClosure");
        }

        // 析构函数，记录析构日志
        ~RpcClosure() {
            INFOLOG("~RpcClosure");
        }

        // 重载的Run方法，如果回调函数不为空，则执行回调函数
        void Run() override {
            if (m_cb) {
                m_cb();
            }
        }

    private:
        // 存储回调函数的std::function对象
        std::function<void()> m_cb {nullptr};
    };

};

#endif