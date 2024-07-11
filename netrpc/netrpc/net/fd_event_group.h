/*
 * @Description: FdEventGroup类头文件，用于管理一组FdEvent对象
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-07 18:23:57
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-07 18:44:39
 */
#ifndef NETRPC_NET_FD_EVENT_GROUP_H
#define NETRPC_NET_FD_EVENT_GROUP_H

#include <vector>
#include <mutex>
#include "netrpc/net/fd_event.h"

namespace netrpc
{

    // FdEventGroup类，用于管理一组FdEvent对象
    class FdEventGroup
    {
    public:
        // 构造函数，根据给定大小创建FdEvent对象组
        FdEventGroup(int size);

        // 析构函数，释放资源
        ~FdEventGroup();

        // 根据文件描述符获取对应的FdEvent对象
        FdEvent *getFdEvent(int fd);

    public:
        // 获取FdEventGroup单例对象
        static FdEventGroup *getFdEventGroup();

    private:
        int m_size{0};                           // FdEvent对象组的大小
        std::vector<FdEvent *> m_fd_event_group; // 存储FdEvent对象的向量
        std::mutex m_mutex;                      // 互斥锁，保护FdEvent对象组的访问
    };
};

#endif