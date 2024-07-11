/*
 * @Description: 定义了 MsgIDUtil 类，用于生成消息 ID
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 21:08:01
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-14 21:23:16
 */
#ifndef NETRPC_COMMON_MSGID_UTIL_H
#define NETRPC_COMMON_MSGID_UTIL_H

#include <string>

namespace netrpc
{

    class MsgIDUtil // 消息 ID 工具类
    {

    public:
        // 静态方法，用于生成消息 ID
        static std::string GenMsgID();
    };

}

#endif