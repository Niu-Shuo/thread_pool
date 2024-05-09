#ifndef NRPC_PROTOCOLS_PROTOCOL_H
#define NRPC_PROTOCOLS_PROTOCOL_H

#include "protocol.pb.h"
#include <ostream>
#include "net/buffer.h"

namespace nrpc
{
    class Context
    {
    public:
        Context() : m_conn_type(ConnType_Short) {}
        virtual ~Context() {}
        virtual std::ostream &print(std::ostream &os) const = 0;

    public:
        // GET
        const ConnType &conn_type() const
        {
            return m_conn_type;
        }

        const RpcStatus &rpc_status() const
        {
            return m_rpc_status;
        }
        const std::string &service() const
        {
            return m_service;
        }
        const std::string &method() const
        {
            return m_method;
        }
        const BufferView &payload_view() const
        {
            return m_payload_view;
        }
        const std::string &payload() const
        {
            return m_payload;
        }

        // SET
        void set_conn_type(const ConnType &conn_type)
        {
            m_conn_type = conn_type;
        }
        void set_rpc_status(const RpcStatus &rpc_status)
        {
            m_rpc_status = rpc_status;
        }
        void set_service(const std::string &service)
        {
            m_service = service;
        }
        void set_method(const std::string &method)
        {
            m_method = method;
        }
        void set_payload_view(const BufferView &payload_view)
        {
            m_payload_view = payload_view;
        }
        void set_payload(const std::string &payload)
        {
            m_payload = payload;
        }

        std::string *mutable_payload()
        {
            return &m_payload;
        }

    public:
        // parse and pack request / response
        // parse_request rd_buf => ctx
        virtual ParseProtocolStatus parse_request(Buffer &rd_buf) = 0;
        // pack_response ctx => wr_buf
        virtual bool pack_response(Buffer &wr_buf) const = 0;
        // pack_request ctx => wr_buf
        virtual bool pack_request(Buffer &wr_buf) const = 0;
        // parse_response rd_buf => ctx
        virtual ParseProtocolStatus parse_response(Buffer &rd_buf) = 0;

    protected:
        ConnType m_conn_type;
        RpcStatus m_rpc_status;
        std::string m_service, m_method;
        BufferView m_payload_view;
        std::string m_payload;
    };

    inline std::ostream &operator<<(std::ostream &os, const Context &ctx)
    {
        return ctx.print(os);
    }

    // Protocol
    // TODO 协议类优化 考虑单例
    class Protocol
    {
    public:
        // server
        // parse_request rd_buf => ctx
        virtual ParseProtocolStatus parse_request(Buffer &rd_buf, Context **ctx) = 0;

        // client
        // get_request_context
        virtual Context *new_context() = 0;

        virtual ~Protocol() {}
        virtual const char *name() = 0;
    };
} // namespace nrpc
#endif