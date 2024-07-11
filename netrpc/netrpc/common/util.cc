/*
 * @Description: util.cc 工具类实现
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 18:57:45
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-03 19:03:33
 */
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include "netrpc/common/util.h"

namespace netrpc
{

    // 全局变量，用于存储进程ID
    static int g_pid = 0;

    // 线程局部变量，用于存储线程ID
    static thread_local int t_thread_id = 0;

    // 获取进程ID
    pid_t getPid()
    {
        if (g_pid != 0)
        {
            return g_pid;
        }
        return getpid();
    }

    // 获取线程ID
    pid_t getThreadId()
    {
        if (t_thread_id != 0)
        {
            return t_thread_id;
        }
        return syscall(SYS_gettid);
    }

    // 获取当前时间，并转换为毫秒
    int64_t getNowMs()
    {
        timeval val;
        gettimeofday(&val, NULL);
        return val.tv_sec * 1000 + val.tv_usec / 1000;
    }

    // 将网络字节序转换为主机字节序的int32_t类型
    int32_t getInt32FromNetByte(const char *buf)
    {
        int32_t re;
        memcpy(&re, buf, sizeof(re));
        return ntohl(re);
    }

    // 获取CPU利用率
    double getCPUUtilization()
    {
        std::ifstream statFile("/proc/stat"); // 打开/proc/stat文件
        std::string line;
        std::getline(statFile, line); // 读取第一行
        statFile.close();

        // 提取各个CPU时间片
        unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
        sscanf(line.c_str(), "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq,
               &steal, &guest, &guest_nice);

        // 计算总的闲置时间和总的非闲置时间
        unsigned long totalIdle = idle + iowait;
        unsigned long totalNonIdle = user + nice + system + irq + softirq + steal;
        unsigned long total = totalIdle + totalNonIdle;

        // 计算CPU利用率
        double utilization = (double)(totalNonIdle * 100) / total;

        return utilization;
    }

    // 获取平均负载
    double getAverageLoad()
    {
        std::ifstream loadFile("/proc/loadavg"); // 打开/proc/loadavg文件
        std::string line;
        std::getline(loadFile, line); // 读取一行
        loadFile.close();

        std::istringstream iss(line);
        double load1, load5, load15;
        iss >> load1 >> load5 >> load15; // 解析平均负载信息

        return load1; // 返回1分钟的平均负载
    }

}