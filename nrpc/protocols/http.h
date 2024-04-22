#ifndef NRPC_BASE_PROTOCOL_H
#define NRPC_BASE_PROTOCOL_H

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
        
    private:
        int m_stage;
        int m_body_size;
        int m_http_method;
        BufferView m_http_url, m_http_version, m_request_line;
        std::map<std::string, std::string> m_headers;
        
    };
} // namespace nrpc


#endif