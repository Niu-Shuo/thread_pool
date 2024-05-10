#ifndef NRPC_PROTOCOLS_HTTP_H
#define NRPC_PROTOCOLS_HTTP_H

#include "protocol.h"
#include <map>
#include <string>

namespace nrpc
{
    class HTTPContext : public Context {
    public:
        HTTPContext() : m_stage(0), m_body_size(0), m_http_method(-1) {}
        virtual ~HTTPContext() {}

        std::ostream& print(std::ostream& os) const;

    public:
        // parse and pack request / response
        // parse_request rd_buf => ctx
        ParseProtocolStatus parse_request(Buffer& rd_buf);
        // pack_response ctx => wr_buf
        bool pack_response(Buffer& wr_buf) const;
        // pack_request ctx => wr_buf
        bool pack_request(Buffer& wr_buf) const;
        // parse_response rd_buf => ctx
        ParseProtocolStatus parse_response(Buffer& rd_buf);

    private:
        ParseProtocolStatus _parse_request_line(Buffer& rd_buf);
        ParseProtocolStatus _parse_response_line(Buffer& rd_buf);
        ParseProtocolStatus _parse_headers(Buffer& rd_buf);
        ParseProtocolStatus _parse_body(Buffer& rd_buf);

    public:
        static const size_t MaxHeadlineSize;
        static const size_t MaxHeaderSize;

    private:
        int m_stage;
        int m_body_size;
        int m_http_method;
        BufferView m_http_url, m_http_version, m_request_line;
        std::map<std::string, std::string> m_headers;
        
    };
    class HTTPProtocol : public Protocol {
    public:
        ~HTTPProtocol() {}
        // 服务端
        // parse_request rd_buf => ctx
        ParseProtocolStatus parse_request(Buffer& rd_buf, Context** ctxx);
        // 客户端
        Context* new_context() { return new HTTPContext(); }

        const char * name() { return "HTTP"; }
    public:
        static const size_t HTTPMethodMaxLen = 7;
    };
} // namespace nrpc


#endif