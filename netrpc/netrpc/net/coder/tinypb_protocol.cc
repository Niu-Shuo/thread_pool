/*
 * @Description: 实现了 TinyPBProtocol 结构体
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-13 19:47:36
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 19:51:02
 */
#include "netrpc/net/coder/tinypb_protocol.h"

namespace netrpc {
    char TinyPBProtocol::PB_START = 0x02;
    char TinyPBProtocol::PB_END = 0x03;
}