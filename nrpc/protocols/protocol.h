#ifndef NRPC_PROTOCOLS_PROTOCOL_H
#define NRPC_PROTOCOLS_PROTOCOL_H

#include "protocol.pb.h"
#include <ostream>
#include "net/buffer.h"

namespace nrpc
{
    // Context上下文类，抽象类
    class Context
    {
    public:
        // 构造函数初始化连接类型为短连接
        Context() : m_conn_type(ConnType_Short) {}
        virtual ~Context() {}
        // 打印上下文信息到输出流，纯虚函数
        virtual std::ostream &print(std::ostream &os) const = 0;

    public:
        // GET
        // 获取连接类型
        const ConnType &conn_type() const
        {
            return m_conn_type;
        }

        // 获取 RPC 状态
        const RpcStatus &rpc_status() const
        {
            return m_rpc_status;
        }

        // 获取服务名
        const std::string &service() const
        {
            return m_service;
        }

        // 获取方法名
        const std::string &method() const
        {
            return m_method;
        }

        // 获取负载视图
        const BufferView &payload_view() const
        {
            return m_payload_view;
        }

        // 获取负载数据
        const std::string &payload() const
        {
            return m_payload;
        }

        // SET
        // 设置连接类型
        void set_conn_type(const ConnType &conn_type)
        {
            m_conn_type = conn_type;
        }

        // 设置 RPC 状态
        void set_rpc_status(const RpcStatus &rpc_status)
        {
            m_rpc_status = rpc_status;
        }

        // 设置服务名
        void set_service(const std::string &service)
        {
            m_service = service;
        }

        // 设置方法名
        void set_method(const std::string &method)
        {
            m_method = method;
        }

        // 设置负载视图
        void set_payload_view(const BufferView &payload_view)
        {
            m_payload_view = payload_view;
        }

        // 设置负载数据
        void set_payload(const std::string &payload)
        {
            m_payload = payload;
        }

        // 获取可变的负载数据
        std::string *mutable_payload()
        {
            return &m_payload;
        }

    public:
        // 解析接收到的请求数据（rd_buf），并将解析得到的信息存储到 Context 对象中（ctx）
        // 将响应数据从 Context 对象中提取出来，并打包成适合发送的格式，然后写入到发送缓冲区（wr_buf）中
        // parse and pack request / response
        // 解析请求
        // parse_request rd_buf => ctx
        virtual ParseProtocolStatus parse_request(Buffer &rd_buf) = 0;
        // 打包响应
        // pack_response ctx => wr_buf
        virtual bool pack_response(Buffer &wr_buf) const = 0;
        // 打包请求
        // pack_request ctx => wr_buf
        virtual bool pack_request(Buffer &wr_buf) const = 0;
        // 解析响应
        // parse_response rd_buf => ctx
        virtual ParseProtocolStatus parse_response(Buffer &rd_buf) = 0;

    protected:
        ConnType m_conn_type;            // 连接类型
        RpcStatus m_rpc_status;          // RPC 状态
        std::string m_service, m_method; // 服务名和方法名
        BufferView m_payload_view;       // 负载视图
        std::string m_payload;           // 负载数据
    };
    // 输出流重载
    inline std::ostream &operator<<(std::ostream &os, const Context &ctx)
    {
        return ctx.print(os);
    }

    // Protocol
    // TODO 协议类优化 考虑单例
    class Protocol
    {
    public:
        // 服务端
        // 解析请求：parse_request rd_buf => ctx
        virtual ParseProtocolStatus parse_request(Buffer &rd_buf, Context **ctx) = 0;

        // 客户端
        // 获取请求上下文：get_request_context
        virtual Context *new_context() = 0;

        virtual ~Protocol() {}
        virtual const char *name() = 0;
    };
} // namespace nrpc
#endif