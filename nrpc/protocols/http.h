#ifndef NRPC_PROTOCOLS_HTTP_H
#define NRPC_PROTOCOLS_HTTP_H

#include "protocol.h"
#include <map>
#include <string>

namespace nrpc
{
    // HTTPContext 类，用于表示 HTTP 上下文
    class HTTPContext : public Context
    {
    public:
        // 构造函数
        HTTPContext() : m_stage(0), m_body_size(0), m_http_method(-1) {}
        // 虚析构函数
        virtual ~HTTPContext() {}

        // 打印函数，将 HTTPContext 信息打印到输出流
        std::ostream &print(std::ostream &os) const;

    public:
        // 解析和打包请求/响应
        // 解析请求消息，从 rd_buf 解析到 ctx
        ParseProtocolStatus parse_request(Buffer &rd_buf);
        // 打包响应消息，从 ctx 打包到 wr_buf
        bool pack_response(Buffer &wr_buf) const;
        // 打包请求消息，从 ctx 打包到 wr_buf
        bool pack_request(Buffer &wr_buf) const;
        // 解析响应消息，从 rd_buf 解析到 ctx
        ParseProtocolStatus parse_response(Buffer &rd_buf);

    private:
        // 私有函数，解析请求行
        ParseProtocolStatus _parse_request_line(Buffer &rd_buf);
        // 私有函数，解析响应行
        ParseProtocolStatus _parse_response_line(Buffer &rd_buf);
        // 私有函数，解析头部
        ParseProtocolStatus _parse_headers(Buffer &rd_buf);
        // 私有函数，解析消息体
        ParseProtocolStatus _parse_body(Buffer &rd_buf);

    public:
        // 常量，最大头行大小
        static const size_t MaxHeadlineSize;
        // 常量，最大头大小
        static const size_t MaxHeaderSize;

    private:
        int m_stage;                                           // 当前解析阶段
        int m_body_size;                                       // 消息体大小
        int m_http_method;                                     // HTTP 方法类型
        BufferView m_http_url, m_http_version, m_request_line; // HTTP URL、版本和请求行
        std::map<std::string, std::string> m_headers;          // 头部字段映射表
    };

    // HTTPProtocol 类，用于表示 HTTP 协议
    class HTTPProtocol : public Protocol
    {
    public:
        // 析构函数
        ~HTTPProtocol() {}
        // 解析请求消息，从 rd_buf 解析到 ctxx
        ParseProtocolStatus parse_request(Buffer &rd_buf, Context **ctxx);
        // 创建新的上下文对象
        Context *new_context() { return new HTTPContext(); }

        // 获取协议名称
        const char *name() { return "HTTP"; }

    public:
        // 常量，HTTP 方法最大长度
        static const size_t HTTPMethodMaxLen = 7;
    };
} // namespace nrpc

#endif