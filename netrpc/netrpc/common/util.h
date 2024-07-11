/*
 * @Description: util.h 工具类声明
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 18:56:56
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-03 19:01:12
 */
#ifndef NETRPC_COMMON_UTIL_H
#define NETRPC_COMMON_UTIL_H

#include <sys/types.h>
#include <unistd.h>

namespace netrpc
{

    // 获取当前进程ID
    pid_t getPid();

    // 获取当前线程ID
    pid_t getThreadId();

    // 获取当前时间（毫秒级）
    int64_t getNowMs();

    // 网络字节序转换为主机字节序
    int32_t getInt32FromNetByte(const char *buf);

    // 获取CPU利用率
    double getCPUUtilization();

    // 获取系统的平均负载能力
    double getAverageLoad();

}

#endif