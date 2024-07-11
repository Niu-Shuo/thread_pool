/*
 * @Description: 定义了 TinyPBCoder 类，实现了 AbstractCoder 的接口，用于 TinyPB 协议的编解码。
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-13 19:48:09
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-07-03 20:53:33
 */

#ifndef NETRPC_NET_CODER_TINYPB_CODER_H
#define NETRPC_NET_CODER_TINYPB_CODER_H

#include "netrpc/net/coder/abstract_coder.h"
#include "netrpc/net/coder/tinypb_protocol.h"

namespace netrpc {

    // 定义 TinyPBCoder 类，继承自 AbstractCoder，实现 TinyPB 协议的编解码功能
    class TinyPBCoder : public AbstractCoder {
    public:
        TinyPBCoder() {} // 默认构造函数
        ~TinyPBCoder() {} // 析构函数

    public:
        // 将 message 对象转化为字节流，写入到 out_buffer 中
        void encode(std::vector<AbstractProtocol::AbstractProtocolPtr>& messages, TcpBuffer::TcpBufferPtr out_buffer);

        // 将 buffer 中的字节流解析为 message 对象，并存储到 out_messages 中
        void decode(std::vector<AbstractProtocol::AbstractProtocolPtr>& out_messages, TcpBuffer::TcpBufferPtr buffer);

    private:
        // 对 TinyPB 协议的特定消息对象进行编码，返回编码后的字节流和长度
        const char* encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int& len);
    };

}; // namespace netrpc

#endif // NETRPC_NET_CODER_TINYPB_CODER_H