#ifndef NRPC_PROTOCOLS_SIMPLE_H
#define NRPC_PROTOCOLS_SIMPLE_H

#include "protocol.h"

namespace nrpc {
    // SimpleContext
    // Simple 协议的上下文类，继承自 Context 类
    class SimpleContext : public Context {
    public:
        virtual ~SimpleContext() {}

        // 构造函数
        SimpleContext() : m_stage(0) {}

        // 打印上下文信息
        std::ostream& print(std::ostream& os) const;

        // 解析请求消息
        ParseProtocolStatus parse_request(Buffer& rd_buf);

        // 打包响应消息
        bool pack_response(Buffer& wr_buf) const;

        // 打包请求消息
        bool pack_request(Buffer& wr_buf) const;

        // 解析响应消息
        ParseProtocolStatus parse_response(Buffer& rd_buf);

    private:
        int m_stage; // Simple 协议的解析阶段
    };

    // SimpleProtocol
    // Simple 协议类，继承自 Protocol 类
    // magic_num | conn_type | body_len | body
    // body => service_len | service | method_len | method | data_len | data
    class SimpleProtocol : public Protocol {
    public:
        // 析构函数
        ~SimpleProtocol() {}

        // 解析请求消息
        ParseProtocolStatus parse_request(Buffer& rd_buf, Context** ctxx);

        // 创建新的上下文对象
        Context* new_context() { return new SimpleContext(); }

        // 获取协议名称
        const char* name() {return "simple_proto"; }

    public:
        static const int MAGIC_NUM = 0x19951028; // Simple 协议的魔数
    };
};  // namespace nrpc

#endif