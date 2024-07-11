/*
 * @Description: RunTime.h 运行时类声明
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 20:14:29
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-03 20:17:17
 */
#ifndef NETRPC_COMMON_RUN_TIME_H
#define NETRPC_COMMON_RUN_TIME_H

#include <string>

namespace netrpc
{
    // 运行时类，用于记录运行时信息
    class RunTime
    {
    public:
        // 获取运行时对象的单例实例
        static RunTime *GetRunTime();

    public:
        std::string m_msgid;       // 消息ID
        std::string m_method_name; // 方法名称
    };
};

#endif