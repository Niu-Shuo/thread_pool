#include "simple.h"
#include "base/logging.h"

namespace nrpc
{

    std::ostream &nrpc::SimpleContext::print(std::ostream &os) const
    {
        // TODO: 在此处插入 return 语句
    }
    // parse_request
    ParseProtocolStatus SimpleContext::parse_request(Buffer &rd_buf)
    {
        while (true) {
            if(m_stage == 0) {
                // parse headers
                if (rd_buf.readable() < sizeof(int)) {
                    return ParseProtocol_NoEnoughData;
                }
                // magic_num
                int magic_num = rd_buf.peek_int32();
                if (magic_num != SimpleProtocol::MAGIC_NUM) {
                    LOG_WARNING << "SimpleContext invalid magic_num: " << magic_num;
                    return ParseProtocol_Error;
                }
                rd_buf.retrieve(sizeof(int));
                m_stage = 1;
            } else if (m_stage == 1) {
                // conn_type
                if (rd_buf.readable() < sizeof(int)) {
                    return ParseProtocol_NoEnoughData;
                }
                int conn_type = rd_buf.peek_int32();
                if (!ConnType_IsValid(conn_type)) {
                    LOG_WARNING << "SimpleContext invalid conn_type: " << m_conn_type;
                    return ParseProtocol_Error;
                }
                rd_buf.retrieve(sizeof(int));
                m_conn_type = ConnType(conn_type);
                m_stage = 2;
            } else if (m_stage == 2) {
                // parse path
                if (rd_buf.readable() < sizeof(uint32_t)) {
                    return ParseProtocol_NoEnoughData;
                }
                // path_len
                uint32_t path_len = rd_buf.peek_uint32();
                if (rd_buf.readable() < path_len) {
                    return ParseProtocol_NoEnoughData;
                }
                rd_buf.retrieve(sizeof(uint32_t));

                const char* tmp = static_cast<char*>(memchr(rd_buf.begin(), '/', path_len));
                if (!tmp) {
                    LOG_WARNING << "SimpleContext failed to parse path";
                    return ParseProtocol_Error;
                }
                m_service = std::string(rd_buf.begin(), tmp - rd_buf.begin());
                m_method = std::string(tmp + 1, path_len  - 1 - (tmp - rd_buf.begin()));
                rd_buf.retrieve(path_len);
                m_stage = 3;
            } else if (m_stage == 3) {
                // parse path
                if (rd_buf.readable() < sizeof(uint32_t)) {
                    return ParseProtocol_NoEnoughData;
                }
                // path_len
                uint32_t payload_len = rd_buf.peek_uint32();
                if (rd_buf.readable() < payload_len) {
                    return ParseProtocol_NoEnoughData;
                }
                rd_buf.retrieve(sizeof(uint32_t));
                m_payload_view = BufferView(rd_buf, payload_len);
                m_stage = 4;
                rd_buf.retrieve(payload_len);
            } else {
                break;
            }
        }
        return ParseProtocol_Success;
    }

    bool SimpleContext::pack_response(Buffer &wr_buf) const
    {
        wr_buf.append(SimpleProtocol::MAGIC_NUM);
        wr_buf.append(m_conn_type);
        wr_buf.append(m_rpc_status);
        uint32_t payload_len = m_payload.length();
        wr_buf.append(payload_len);
        wr_buf.append_extra(std::move(m_payload));
        return true;
    }

    bool SimpleContext::pack_request(Buffer &wr_buf) const
    {
        wr_buf.append(SimpleProtocol::MAGIC_NUM);
        wr_buf.append(m_conn_type);

        uint32_t path_len = m_service.length() + 1 + m_method.length();
        wr_buf.append(path_len);
        wr_buf.append(m_service);
        wr_buf.append("/");
        wr_buf.append(m_method);

        uint32_t payload_len = m_payload.length();
        wr_buf.append(payload_len);
        wr_buf.append_extra(std::move(m_payload));
        return true;
    }
    // parse_request
    ParseProtocolStatus SimpleProtocol::parse_request(Buffer &rd_buf, Context **ctxx)
    {   
        if (!ctxx) { return ParseProtocol_Error; }
    }
}; // namespace nrpc