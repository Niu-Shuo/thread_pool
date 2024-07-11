/*
 * @Description: 定义了 TinyPBProtocol 结构体，用于描述 TinyPB 协议的数据结构和成员变量。
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-13 19:39:43
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 19:50:47
 */

#ifndef NETRPC_NET_CODER_TINYPB_PROTOCOL_H
#define NETRPC_NET_CODER_TINYPB_PROTOCOL_H

#include <string>
#include "netrpc/net/coder/abstract_protocol.h"

namespace netrpc
{

    // 定义 TinyPBProtocol 结构体，继承自 AbstractProtocol
    struct TinyPBProtocol : public AbstractProtocol
    {
    public:
        TinyPBProtocol() {}  // 默认构造函数
        ~TinyPBProtocol() {} // 析构函数

    public:
        static char PB_START; // TinyPB 协议的起始字符
        static char PB_END;   // TinyPB 协议的结束字符

    public:
        int32_t m_pk_len{0};     // 包长度
        int32_t m_msg_id_len{0}; // 消息ID长度，继承自父类

        int32_t m_method_name_len{0}; // 方法名长度
        std::string m_method_name;    // 方法名
        int32_t m_err_code{0};        // 错误码
        int32_t m_err_info_len{0};    // 错误信息长度
        std::string m_err_info;       // 错误信息
        std::string m_pb_data;        // PB 数据
        int32_t m_check_sum{0};       // 校验和

        bool parse_success{false}; // 解析成功标志
    };

}; // namespace netrpc

#endif // NETRPC_NET_CODER_TINYPB_PROTOCOL_H