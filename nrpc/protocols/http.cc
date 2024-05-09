#include "http.h"
#include "base/logging.h"

namespace nrpc {
    const size_t HTTPContext::MaxHeadlineSize = 65536;
    const size_t HTTPContext::MaxHeaderSize = 65536;
    std::ostream& HTTPContext::print(std::ostream& os) const {
        os << "HTTPContext:"
        << "\nstage: " << m_stage
        << "\nrequest_line: " << m_request_line.str()
        << "\nservice: " << m_service
        << "\nmethod: " << m_method
        << "\nheaders: ";
        for (const auto& header : m_headers) {
            os << "\n    " << header.first << ": " << header.second;
        }
        return os;
    }
    // parse and pack request / response
    // parse_request rd_buf => ctx
    ParseProtocolStatus HTTPContext::parse_request(Buffer& rd_buf) {
        while (true) {
            if (m_stage == 0) {
                ParseProtocolStatus res = _parse_request_line(rd_buf);
                if (res == ParseProtocol_PartSuccess) {
                    ++m_stage;
                } else {
                    return res;
                }
            } else if (m_stage == 1) {
                ParseProtocolStatus res = _parse_headers(rd_buf);
                if (res == ParseProtocol_PartSuccess) {
                    ++m_stage;
                } else {
                    return res;
                }
            } else if (m_stage == 2) {
                ParseProtocolStatus res = _parse_body(rd_buf);
                if (res == ParseProtocol_PartSuccess) {
                    ++m_stage;
                } else {
                    return res;
                }
            } else {
                return ParseProtocol_Success;
            }
        }
        return ParseProtocol_Success;
    }
    // pack_response ctx => wr_buf
    bool HTTPContext::pack_response(Buffer &wr_buf) const
    {
        wr_buf.append("HTTP/1.1 200 OK\r\n");
        if (m_conn_type == ConnType_Short) {
            wr_buf.append("Connection: Close\r\n");
        } else {
            wr_buf.append("Connection: Keep-Alive\r\n");
        }
        wr_buf.append("Content-Type: application/data\r\n");
        wr_buf.append("Content-Length: ");
        wr_buf.append(std::to_string(m_payload.length()));
        wr_buf.append("\r\n\r\n");
        wr_buf.append_extra(std::move(m_payload));
        return true;
    }
    // pack_request ctx => wr_buf
    bool HTTPContext::pack_request(Buffer &wr_buf) const
    {
        wr_buf.append("POST /");
        wr_buf.append(m_service);
        wr_buf.append("/");
        wr_buf.append(m_method);
        wr_buf.append(" HTTP/1.1\r\n");
        if (m_conn_type == ConnType_Short) {
            wr_buf.append("Connection: Close\r\n");
        } else {
            wr_buf.append("Connection: Keep-Alive\r\n");
        }
        wr_buf.append("Content-Type: application/data\r\n");
        // wr_buf.append("Content-Type: application/json");
        wr_buf.append("Content-Length: ");
        wr_buf.append(std::to_string(m_payload.length()));
        wr_buf.append("\r\n\r\n");
        wr_buf.append_extra(std::move(m_payload));
        return true;
    }
    // _parse_request_line
    ParseProtocolStatus HTTPContext::_parse_request_line(Buffer& rd_buf) {
        size_t size = std::min(rd_buf.readable(), MaxHeadlineSize);
        if (size == 0) {
            return ParseProtocol_NoEnoughData;
        }
        // find CRLF
        int n = rd_buf.find_crlf(size);
        if (n == -1) {
            return (size == MaxHeadlineSize) ? ParseProtocol_Error : ParseProtocol_NoEnoughData;
        }

        // http method
        int offset = 0;
        for (int i = HTTPMethod::_HTTPMethod_MIN; i < HTTPMethod::_HTTPMethod_MAX; ++i) {
            const std::string& method = HTTPMethod::_HTTPMethod_Name(i);
            if (strncmp(rd_buf.begin(), method.data(), method.length()) == 0) {
                m_http_method = HTTPMethod::_HTTPMethod(i);
                offset += method.length();
                break;
            }
        }
        if (!HTTPMethod::_HTTPMethod_IsValid(m_http_method)) { return ParseProtocol_Error; }
        if (*(rd_buf.begin() + offset) != ' ') { return ParseProtocol_Error; }
        ++offset;

        // http url
        char* tmp = static_cast<char*>(memchr(rd_buf.begin() + offset, ' ', n - offset));
        if (tmp == nullptr || tmp == (rd_buf.begin() + offset) || *(rd_buf.begin() + offset) != '/') {
            return ParseProtocol_Error;
        }
        m_http_url = BufferView(rd_buf, rd_buf.begin() + offset, tmp - rd_buf.begin() - offset);
        char * tmp2 = static_cast<char*>(memchr(rd_buf.begin() + offset + 1, '/', tmp - rd_buf.begin() - offset));
        if (tmp2) {
            m_service = std::string(rd_buf.begin() + offset + 1, tmp2 - rd_buf.begin() - offset - 1);
            m_method = std::string(tmp2 + 1, tmp - tmp2 - 1);
        }
        offset = tmp - rd_buf.begin() + 1;

        // http version
        tmp = static_cast<char*>(memchr(rd_buf.begin() + offset, ' ', n - offset));
        if (tmp) {
            return ParseProtocol_Error;
        }
        m_http_version = BufferView(rd_buf, rd_buf.begin() + offset, n - offset);

        // request line
        m_request_line = BufferView(rd_buf, rd_buf.begin(), n);
        rd_buf.retrieve(n + 2);
        return ParseProtocol_PartSuccess;
    }
    // _parse_response_line
    ParseProtocolStatus HTTPContext::_parse_response_line(Buffer& rd_buf) {
        size_t size = std::min(rd_buf.readable(), MaxHeadlineSize);
        if (size == 0) {
            return ParseProtocol_NoEnoughData;
        }
        // find CRLF
        int n = rd_buf.find_crlf(size);
        if (n == -1) {
            return (size == MaxHeadlineSize) ? ParseProtocol_Error : ParseProtocol_NoEnoughData;
        }
        // request line
        m_request_line = BufferView(rd_buf, rd_buf.begin(), n);
        rd_buf.retrieve(n + 2);

        return ParseProtocol_PartSuccess;
    }
    // std::string trim
    static void trim(std::string& str) {
        str.erase(0, str.find_first_not_of(" "));
        str.erase(str.find_first_not_of(" ") + 1);
    }
    // _parse_headers
    ParseProtocolStatus HTTPContext::_parse_headers(Buffer& rd_buf) {
        while (true) {
            size_t size = std::min(rd_buf.readable(), MaxHeaderSize);
            if (size == 0) {
                return ParseProtocol_NoEnoughData;
            }

            // find CRLF
            int n = rd_buf.find_crlf(size);
            if (n == -1) {
                return (size == MaxHeadlineSize) ? ParseProtocol_Error : ParseProtocol_NoEnoughData;
            }
            if (n > 0) {
                const char* tmp = static_cast<char*>(memchr(rd_buf.begin(), ':', n));
                if (!tmp || tmp == rd_buf.begin()) {
                    LOG_WARNING << "HTTPContext failed to parse header " << std::string(rd_buf.begin(), n);
                    return ParseProtocol_Error;
                }
                std::string key(rd_buf.begin(), tmp - rd_buf.begin());
                std::string value(tmp + 1, rd_buf.begin() - tmp + n - 1);
                trim(key), trim(value);
                if (!key.empty() && !value.empty() && m_headers.find(key) == m_headers.end()) {
                    if (key == "Content-Length") {
                        m_body_size = std::stoi(value);
                    }
                    if (key == "Connection" && value == "Keep-Alive") {
                        m_conn_type = ConnType_Single;
                    }
                    m_headers[key] = std::move(value);
                }
                rd_buf.retrieve(n + 2);
            } else if (n == 0) {
                rd_buf.retrieve(2);
                return ParseProtocol_PartSuccess;
            }
        }
        return ParseProtocol_PartSuccess;
    }
    // _parse_body
    ParseProtocolStatus HTTPContext::_parse_body(Buffer& rd_buf) {
        if (static_cast<int>(rd_buf.readable()) >= m_body_size) {
            m_payload_view = BufferView(rd_buf, rd_buf.begin(), m_body_size);
            return ParseProtocol_PartSuccess;
        }
        return ParseProtocol_NoEnoughData;
    }
    ParseProtocolStatus HTTPProtocol::parse_request(Buffer &rd_buf, Context **ctxx)
    {
        return ParseProtocolStatus();
    }
}; // namespace nrpc