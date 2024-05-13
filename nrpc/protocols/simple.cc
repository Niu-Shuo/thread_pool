#include "simple.h"
#include "base/logging.h"

namespace nrpc
{

    std::ostream &nrpc::SimpleContext::print(std::ostream &os) const
    {
        // 打印 SimpleContext 的信息，包括服务名、方法名和有效载荷大小
        os << "SimpleContext. service: " << m_service
           << ", method: " << m_method
           << ", payload_size: " << m_payload_view.size();
        return os;
    }

    // 解析请求消息
    ParseProtocolStatus SimpleContext::parse_request(Buffer &rd_buf)
    {
        // 循环解析消息的不同部分
        while (true)
        {
            if (m_stage == 0)
            {
                // 解析消息头部
                if (rd_buf.readable() < sizeof(int))
                {
                    return ParseProtocol_NoEnoughData;
                }
                // 魔数
                int magic_num = rd_buf.peek_int32();
                if (magic_num != SimpleProtocol::MAGIC_NUM)
                {
                    LOG_WARNING << "SimpleContext invalid magic_num: " << magic_num;
                    return ParseProtocol_Error;
                }
                rd_buf.retrieve(sizeof(int));
                m_stage = 1;
            }
            else if (m_stage == 1)
            {
                // 连接类型
                if (rd_buf.readable() < sizeof(int))
                {
                    return ParseProtocol_NoEnoughData;
                }
                int conn_type = rd_buf.peek_int32();
                if (!ConnType_IsValid(conn_type))
                {
                    LOG_WARNING << "SimpleContext invalid conn_type: " << m_conn_type;
                    return ParseProtocol_Error;
                }
                rd_buf.retrieve(sizeof(int));
                m_conn_type = ConnType(conn_type);
                m_stage = 2;
            }
            else if (m_stage == 2)
            {
                // 解析路径
                if (rd_buf.readable() < sizeof(uint32_t))
                {
                    return ParseProtocol_NoEnoughData;
                }
                // 路径长度
                uint32_t path_len = rd_buf.peek_uint32();
                if (rd_buf.readable() < path_len)
                {
                    return ParseProtocol_NoEnoughData;
                }
                rd_buf.retrieve(sizeof(uint32_t));

                const char *tmp = static_cast<char *>(memchr(rd_buf.begin(), '/', path_len));
                if (!tmp)
                {
                    LOG_WARNING << "SimpleContext failed to parse path";
                    return ParseProtocol_Error;
                }
                m_service = std::string(rd_buf.begin(), tmp - rd_buf.begin());
                m_method = std::string(tmp + 1, path_len - 1 - (tmp - rd_buf.begin()));
                rd_buf.retrieve(path_len);
                m_stage = 3;
            }
            else if (m_stage == 3)
            {
                // 解析有效载荷
                if (rd_buf.readable() < sizeof(uint32_t))
                {
                    return ParseProtocol_NoEnoughData;
                }
                // 有效载荷长度
                uint32_t payload_len = rd_buf.peek_uint32();
                if (rd_buf.readable() < payload_len)
                {
                    return ParseProtocol_NoEnoughData;
                }
                rd_buf.retrieve(sizeof(uint32_t));
                m_payload_view = BufferView(rd_buf, payload_len);
                m_stage = 4;
                rd_buf.retrieve(payload_len);
            }
            else
            {
                break;
            }
        }
        return ParseProtocol_Success;
    }

    // 打包响应消息
    bool SimpleContext::pack_response(Buffer &wr_buf) const
    {
        // 添加消息头部和连接类型
        wr_buf.append(SimpleProtocol::MAGIC_NUM);
        wr_buf.append(m_conn_type);
        // 添加 RPC 状态码和有效载荷长度
        wr_buf.append(m_rpc_status);
        uint32_t payload_len = m_payload.length();
        wr_buf.append(payload_len);
        // 添加有效载荷内容
        wr_buf.append_extra(std::move(m_payload));
        return true;
    }

    // 打包请求消息
    bool SimpleContext::pack_request(Buffer &wr_buf) const
    {
        // 添加消息头部和连接类型
        wr_buf.append(SimpleProtocol::MAGIC_NUM);
        wr_buf.append(m_conn_type);
        // 添加服务名和方法名长度
        uint32_t path_len = m_service.length() + 1 + m_method.length();
        wr_buf.append(path_len);
        // 添加服务名、路径分隔符和方法名
        wr_buf.append(m_service);
        wr_buf.append("/");
        wr_buf.append(m_method);
        // 添加有效载荷长度和内容
        uint32_t payload_len = m_payload.length();
        wr_buf.append(payload_len);
        wr_buf.append_extra(std::move(m_payload));
        return true;
    }

    // 解析响应消息
    ParseProtocolStatus SimpleContext::parse_response(Buffer &rd_buf)
    {
        // 循环解析消息的不同部分
        while (true)
        {
            if (m_stage == 0)
            {
                // 解析消息头部
                if (rd_buf.readable() < sizeof(int))
                {
                    return ParseProtocol_NoEnoughData;
                }
                // 魔数
                int magic_num = rd_buf.peek_int32();
                if (magic_num != SimpleProtocol::MAGIC_NUM)
                {
                    LOG_WARNING << "SimpleContext invalid magic_num: " << magic_num;
                    return ParseProtocol_Error;
                }
                rd_buf.retrieve(sizeof(int));
                m_stage = 1;
            }
            else if (m_stage == 1)
            {
                // 连接类型和 RPC 状态码
                if (rd_buf.readable() < 2 * sizeof(int))
                {
                    return ParseProtocol_NoEnoughData;
                }
                int conn_type = rd_buf.peek_int32();
                if (!ConnType_IsValid(conn_type))
                {
                    LOG_WARNING << "SimpleContext invalid conn_type: " << m_conn_type;
                    return ParseProtocol_Error;
                }
                rd_buf.retrieve(sizeof(int));
                m_conn_type = ConnType(conn_type);
                // RPC 状态码
                int rpc_status = rd_buf.peek_int32();
                if (!RpcStatus_IsValid(rpc_status))
                {
                    LOG_WARNING << "SimpleContext invalid rpc_status: " << rpc_status;
                    return ParseProtocol_Error;
                }
                rd_buf.retrieve(sizeof(int));
                m_rpc_status = RpcStatus(rpc_status);
                m_stage = 2;
            }
            else if (m_stage == 2)
            {
                // 解析有效载荷
                if (rd_buf.readable() < sizeof(uint32_t))
                {
                    return ParseProtocol_NoEnoughData;
                }
                // 有效载荷长度
                uint32_t payload_len = rd_buf.peek_uint32();
                if (rd_buf.readable() < payload_len)
                {
                    return ParseProtocol_NoEnoughData;
                }
                rd_buf.retrieve(sizeof(uint32_t));
                m_payload_view = BufferView(rd_buf, payload_len);
                m_stage = 3;
                rd_buf.retrieve(payload_len);
            }
            else
            {
                break;
            }
        }
        return ParseProtocol_Success;
    }

    // 解析请求消息
    ParseProtocolStatus SimpleProtocol::parse_request(Buffer &rd_buf, Context **ctxx)
    {
        // 检查上下文指针是否为空
        if (!ctxx)
        {
            return ParseProtocol_Error;
        }
        // 如果上下文指针为空，则尝试解析协议
        if (*ctxx == nullptr)
        {
            if (rd_buf.readable() < sizeof(MAGIC_NUM))
            {
                return ParseProtocol_NoEnoughData;
            }
            if (rd_buf.peek_int32() != MAGIC_NUM)
            {
                return ParseProtocol_TryAnotherProtocol;
            }
            *ctxx = new SimpleContext();
        }
        // 将上下文指针转换为 SimpleContext 指针，并进行解析请求
        SimpleContext *ctx = dynamic_cast<SimpleContext *>(*ctxx);
        if (!ctx)
        {
            LOG_WARNING << "SimpleProtocol failed to dynamic_cast SimpleContext.";
            return ParseProtocol_Error;
        }
        return ctx->parse_request(rd_buf);
    }
}; // namespace nrpc