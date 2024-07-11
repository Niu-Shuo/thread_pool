/*
 * @Description: 
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-07 21:20:38
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-08 21:32:04
 */
#ifndef NETRPC_NET_CODER_STRING_CODER_H
#define NETRPC_NET_CODER_STRING_CODER_H

#include "netrpc/net/coder/abstract_coder.h"
#include "netrpc/net/coder/abstract_protocol.h"

namespace netrpc {
    class StringProtocol : public AbstractProtocol {
    public:
        std::string info;
    };

    class StringCoder : public AbstractCoder {
        // 将 message 对象转化为字节流，写入到 buffer 
        void encode(std::vector<AbstractProtocol::AbstractProtocolPtr>& message, TcpBuffer::TcpBufferPtr out_buffer) {
            for (size_t ii = 0; ii < message.size(); ++ii) {
                std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(message[ii]);
                out_buffer->writeToBuffer(msg->info.c_str(), msg->info.length());
            }
        }

        // 将 buffer 里面的字节流转换为 message 对象
        void decode(std::vector<AbstractProtocol::AbstractProtocolPtr>& out_message, TcpBuffer::TcpBufferPtr buffer) {
            std::vector<char> re;
            buffer->readFromBuffer(re, buffer->readAble());
            std::string info;
            for (size_t i = 0; i < re.size(); ++i) {
                info += re[i];
            }

            std::shared_ptr<StringProtocol> msg = std::make_shared<StringProtocol>();
            msg->info = info;
            msg->m_msg_id = "123456";   // 随机数
            out_message.push_back(msg);
        }
    };
};

#endif