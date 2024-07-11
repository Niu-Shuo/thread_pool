/*
 * @Description: IOThreadGroup类定义，管理一组IO线程
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-07 17:22:48
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-07 17:44:07
 */
#ifndef NETRPC_NET_IO_THREAD_GROUP_H
#define NETRPC_NET_IO_THREAD_GROUP_H

#include <vector>
#include "netrpc/common/log.h"
#include "netrpc/net/io_thread.h"

namespace netrpc {

    // IOThreadGroup类，用于管理一组IO线程
    class IOThreadGroup {
    public:
        // 构造函数，接受线程组大小作为参数
        IOThreadGroup(int size);

        // 析构函数，清理资源
        ~IOThreadGroup();

        // 启动所有IO线程
        void start();

        // 等待所有IO线程结束
        void join();

        // 获取一个IO线程
        IOThread* getIOThread();

    private:
        int m_size {0}; // IO线程组的大小
        std::vector<IOThread*> m_io_thread_groups; // 存储IO线程的向量
        int m_index {0}; // 用于轮询获取IO线程的索引
    };
};

#endif