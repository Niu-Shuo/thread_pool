#include "http.h"
#include "base/logging.h"

namespace nrpc
{
    // 初始化静态成员变量
    const size_t HTTPContext::MaxHeadlineSize = 65536;
    const size_t HTTPContext::MaxHeaderSize = 65536;

    // 打印函数，将 HTTPContext 信息打印到输出流
    std::ostream &HTTPContext::print(std::ostream &os) const
    {
        os << "HTTPContext:"
           << "\nstage: " << m_stage
           << "\nrequest_line: " << m_request_line.str()
           << "\nservice: " << m_service
           << "\nmethod: " << m_method
           << "\nheaders: ";
        for (const auto &header : m_headers)
        {
            os << "\n    " << header.first << ": " << header.second;
        }
        return os;
    }

    // 解析请求消息，从 rd_buf 解析到 ctx
    // parse and pack request / response
    // parse_request rd_buf => ctx
    ParseProtocolStatus HTTPContext::parse_request(Buffer &rd_buf)
    {
        while (true)
        {
            if (m_stage == 0)
            {
                // 解析请求行
                ParseProtocolStatus res = _parse_request_line(rd_buf);
                if (res == ParseProtocol_PartSuccess) { ++m_stage; }
                else { return res; }
            }
            else if (m_stage == 1)
            {
                // 解析头部
                ParseProtocolStatus res = _parse_headers(rd_buf);
                if (res == ParseProtocol_PartSuccess) { ++m_stage; }
                else { return res; }
            }
            else if (m_stage == 2)
            {
                // 解析消息体
                ParseProtocolStatus res = _parse_body(rd_buf);
                if (res == ParseProtocol_PartSuccess) { ++m_stage; }
                else { return res; }
            }
            else { return ParseProtocol_Success; }
        }
        return ParseProtocol_Success;
    }

    // 打包响应消息，从 ctx 打包到 wr_buf
    // pack_response ctx => wr_buf
    bool HTTPContext::pack_response(Buffer &wr_buf) const
    {
        // 构建响应消息头部
        wr_buf.append("HTTP/1.1 200 OK\r\n");
        if (m_conn_type == ConnType_Short)
        {
            wr_buf.append("Connection: Close\r\n");
        }
        else
        {
            wr_buf.append("Connection: Keep-Alive\r\n");
        }
        wr_buf.append("Content-Type: application/data\r\n");
        wr_buf.append("Content-Length: ");
        wr_buf.append(std::to_string(m_payload.length()));
        wr_buf.append("\r\n\r\n");
        // 添加消息体
        wr_buf.append_extra(std::move(m_payload));
        return true;
    }

    // 打包请求消息，从 ctx 打包到 wr_buf
    // pack_request ctx => wr_buf
    bool HTTPContext::pack_request(Buffer &wr_buf) const
    {
        // 构建请求消息头部
        wr_buf.append("POST /");
        wr_buf.append(m_service);
        wr_buf.append("/");
        wr_buf.append(m_method);
        wr_buf.append(" HTTP/1.1\r\n");
        if (m_conn_type == ConnType_Short)
        {
            wr_buf.append("Connection: Close\r\n");
        }
        else
        {
            wr_buf.append("Connection: Keep-Alive\r\n");
        }
        wr_buf.append("Content-Type: application/data\r\n");
        // wr_buf.append("Content-Type: application/json");
        wr_buf.append("Content-Length: ");
        wr_buf.append(std::to_string(m_payload.length()));
        wr_buf.append("\r\n\r\n");
        // 添加消息体
        wr_buf.append_extra(std::move(m_payload));
        return true;
    }

    // 解析请求行
    ParseProtocolStatus HTTPContext::_parse_request_line(Buffer &rd_buf)
    {
        // 获取可读取的数据长度
        size_t size = std::min(rd_buf.readable(), MaxHeadlineSize);
        if (size == 0)
        {
            return ParseProtocol_NoEnoughData; // 数据不足
        }
        // 查找 CRLF
        int n = rd_buf.find_crlf(size);
        if (n == -1)
        {
            // 如果未找到 CRLF，则根据数据是否达到最大头部大小返回错误或者需要更多数据
            return (size == MaxHeadlineSize) ? ParseProtocol_Error : ParseProtocol_NoEnoughData;
        }

        // 解析 HTTP 方法
        int offset = 0;
        for (int i = HTTPMethod::_HTTPMethod_MIN; i < HTTPMethod::_HTTPMethod_MAX; ++i)
        {
            const std::string &method = HTTPMethod::_HTTPMethod_Name(i);
            if (strncmp(rd_buf.begin(), method.data(), method.length()) == 0)
            {
                m_http_method = HTTPMethod::_HTTPMethod(i);
                offset += method.length();
                break;
            }
        }

        // 判断 HTTP 方法是否有效
        if (!HTTPMethod::_HTTPMethod_IsValid(m_http_method))
        {
            return ParseProtocol_Error; // 返回解析错误
        }
        if (*(rd_buf.begin() + offset) != ' ')
        {
            return ParseProtocol_Error; // 返回解析错误
        }
        ++offset;

        // 解析 URL
        char *tmp = static_cast<char *>(memchr(rd_buf.begin() + offset, ' ', n - offset));
        if (tmp == nullptr || tmp == (rd_buf.begin() + offset) || *(rd_buf.begin() + offset) != '/')
        {
            return ParseProtocol_Error; // 返回解析错误
        }
        m_http_url = BufferView(rd_buf, rd_buf.begin() + offset, tmp - rd_buf.begin() - offset);
        char *tmp2 = static_cast<char *>(memchr(rd_buf.begin() + offset + 1, '/', tmp - rd_buf.begin() - offset));
        if (tmp2)
        {
            // 解析服务和方法
            m_service = std::string(rd_buf.begin() + offset + 1, tmp2 - rd_buf.begin() - offset - 1);
            m_method = std::string(tmp2 + 1, tmp - tmp2 - 1);
        }
        offset = tmp - rd_buf.begin() + 1;

        // 解析 HTTP 版本
        tmp = static_cast<char *>(memchr(rd_buf.begin() + offset, ' ', n - offset));
        if (tmp)
        {
            return ParseProtocol_Error; // 返回解析错误
        }
        m_http_version = BufferView(rd_buf, rd_buf.begin() + offset, n - offset);

        // 获取请求行
        m_request_line = BufferView(rd_buf, rd_buf.begin(), n);
        rd_buf.retrieve(n + 2);           // 移除已解析的数据
        return ParseProtocol_PartSuccess; // 返回部分解析成功
    }

    // 解析消息头部
    // _parse_response_line
    ParseProtocolStatus HTTPContext::_parse_response_line(Buffer &rd_buf)
    {
        size_t size = std::min(rd_buf.readable(), MaxHeadlineSize);
        if (size == 0) { return ParseProtocol_NoEnoughData; }
        // 查找 CRLF
        int n = rd_buf.find_crlf(size);
        if (n == -1)
        {
            return (size == MaxHeadlineSize) ? ParseProtocol_Error : ParseProtocol_NoEnoughData;
        }
        // 解析头部字段
        m_request_line = BufferView(rd_buf, rd_buf.begin(), n);
        rd_buf.retrieve(n + 2);

        return ParseProtocol_PartSuccess;
    }
    // 除字符串 str 的首尾空格
    static void trim(std::string &str)
    {
        str.erase(0, str.find_first_not_of(" "));
        str.erase(str.find_first_not_of(" ") + 1);
    }

    // 解析消息头部
    ParseProtocolStatus HTTPContext::_parse_headers(Buffer &rd_buf)
    {
        while (true)
        {
            // 确定可读取数据的大小，并设置最大头部大小
            size_t size = std::min(rd_buf.readable(), MaxHeaderSize);
            if (size == 0)
            {
                // 如果没有足够的数据可读，则返回需要更多数据
                return ParseProtocol_NoEnoughData;
            }

            // 查找 CRLF
            int n = rd_buf.find_crlf(size);
            if (n == -1)
            {
                // 如果未找到 CRLF，且已达到最大头部大小，则返回解析错误；否则返回需要更多数据
                return (size == MaxHeadlineSize) ? ParseProtocol_Error : ParseProtocol_NoEnoughData;
            }

            // 解析头部字段
            if (n > 0)
            {
                const char *tmp = static_cast<char *>(memchr(rd_buf.begin(), ':', n));
                if (!tmp || tmp == rd_buf.begin())
                {
                    // 如果未找到头部字段的冒号，或者字段为空，则记录警告并返回解析错误
                    LOG_WARNING << "HTTPContext failed to parse header " << std::string(rd_buf.begin(), n);
                    return ParseProtocol_Error;
                }

                // 提取头部字段的键值对
                std::string key(rd_buf.begin(), tmp - rd_buf.begin());
                std::string value(tmp + 1, rd_buf.begin() - tmp + n - 1);

                // 去除键和值两端的空白字符
                trim(key), trim(value);

                // 如果键和值均不为空，并且键不存在于已解析的头部中，则添加到头部映射中
                if (!key.empty() && !value.empty() && m_headers.find(key) == m_headers.end())
                {
                    // 如果键是 Content-Length，则解析并存储消息体大小
                    if (key == "Content-Length")
                    {
                        m_body_size = std::stoi(value);
                    }
                    // 如果键是 Connection 并且值是 Keep-Alive，则设置连接类型为单连接
                    if (key == "Connection" && value == "Keep-Alive")
                    {
                        m_conn_type = ConnType_Single;
                    }
                    // 将键值对添加到头部映射中
                    m_headers[key] = std::move(value);
                }

                // 从缓冲区中移除已解析的头部字段
                rd_buf.retrieve(n + 2);
            }
            else if (n == 0)
            {
                // 如果遇到空行，则表示头部解析完成，从缓冲区中移除空行并返回部分解析成功
                rd_buf.retrieve(2);
                return ParseProtocol_PartSuccess;
            }
        }
        // 返回部分解析成功
        return ParseProtocol_PartSuccess;
    }

    // 解析消息体
    ParseProtocolStatus HTTPContext::_parse_body(Buffer &rd_buf)
    {
        // 检查可读取的数据是否足够解析消息体
        if (static_cast<int>(rd_buf.readable()) >= m_body_size)
        {
            // 如果可读取的数据长度大于等于消息体大小，则将消息体视图设置为缓冲区中的数据
            m_payload_view = BufferView(rd_buf, rd_buf.begin(), m_body_size);
            return ParseProtocol_PartSuccess; // 返回部分解析成功
        }
        return ParseProtocol_NoEnoughData; // 返回需要更多数据
    }

    // 解析请求
    ParseProtocolStatus HTTPProtocol::parse_request(Buffer &rd_buf, Context **ctxx)
    {
        // 检查指针是否有效
        if (!ctxx)
        {
            // 指针无效，返回解析错误
            return ParseProtocol_Error;
        }

        // 如果上下文指针为空，则创建新的上下文对象
        if (*ctxx == nullptr)
        {
            // 检查缓冲区是否包含足够的数据以识别 HTTP 方法
            if (rd_buf.readable() < HTTPMethodMaxLen)
            {
                // 数据不足以识别 HTTP 方法，返回需要更多数据
                return ParseProtocol_NoEnoughData;
            }

            // 遍历所有 HTTP 方法
            for (int ii = HTTPMethod::_HTTPMethod_MIN; ii < HTTPMethod::_HTTPMethod_MAX; ++ii)
            {
                // 获取当前 HTTP 方法的名称
                const std::string &method = HTTPMethod::_HTTPMethod_Name(ii);

                // 检查缓冲区中的数据是否以当前 HTTP 方法开头
                if (strncmp(rd_buf.begin(), method.data(), method.length()) == 0)
                {
                    // 数据与当前 HTTP 方法匹配，创建新的 HTTP 上下文对象
                    *ctxx = new HTTPContext();
                    break;
                }
            }
        }

        // 如果无法识别 HTTP 方法，则尝试其他协议
        if (*ctxx == nullptr)
        {
            return ParseProtocol_TryAnotherProtocol;
        }

        // 将上下文指针转换为 HTTPContext 类型
        HTTPContext *ctx = dynamic_cast<HTTPContext *>(*ctxx);
        if (!ctx)
        {
            // 转换失败，记录警告并返回解析错误
            LOG_WARNING << "HTTPProtocol failed to dynamic_cast HTTPContext.";
            return ParseProtocol_Error;
        }

        // 调用 HTTPContext 的解析请求方法
        return ctx->parse_request(rd_buf);
    }
}; // namespace nrpc