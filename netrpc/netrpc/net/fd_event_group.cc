/*
 * @Description: FdEventGroup类实现文件，用于管理一组FdEvent对象
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-07 18:23:57
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-07-03 19:40:19
 */
#include "netrpc/net/fd_event_group.h"
#include "netrpc/common/log.h"

namespace netrpc
{

    // 全局静态指针，指向FdEventGroup的单例对象
    static FdEventGroup *g_fd_event_group = nullptr;

    // 构造函数，根据给定大小创建FdEvent对象组
    FdEventGroup::FdEventGroup(int size) : m_size(size)
    {
        // 根据给定大小创建对应数量的FdEvent对象
        for (int ii = 0; ii < m_size; ++ii)
        {
            m_fd_event_group.push_back(new FdEvent(ii));
        }
    }

    // 析构函数，释放资源
    FdEventGroup::~FdEventGroup()
    {
        // 删除所有FdEvent对象
        // for (auto &fdEvent_ptr : m_fd_event_group)
        // {
        //     if (fdEvent_ptr != nullptr)
        //     {
        //         delete fdEvent_ptr;
        //         fdEvent_ptr = nullptr;
        //     }
        // }
        for (int ii = 0; ii < m_size; ++ii) {
            if (m_fd_event_group[ii] != nullptr) {
                delete m_fd_event_group[ii];
                m_fd_event_group[ii] = nullptr;
            }
        }
    }

    // 根据文件描述符获取对应的FdEvent对象
    FdEvent *FdEventGroup::getFdEvent(int fd)
    {
        std::lock_guard<std::mutex> lock(m_mutex); // 上锁，保护数据访问
        if ((size_t)fd < m_fd_event_group.size())
        { // 如果已存在对应的FdEvent对象
            return m_fd_event_group[fd];
        }
        // 如果不存在对应的FdEvent对象，则动态调整大小并创建新对象
        int new_size = int(fd * 1.5); // 新大小为原大小的1.5倍
        for (int ii = m_fd_event_group.size(); ii < new_size; ++ii)
        {
            m_fd_event_group.push_back(new FdEvent(ii)); // 创建新的FdEvent对象
        }
        return m_fd_event_group[fd];
    }

    // 获取FdEventGroup单例对象
    FdEventGroup *FdEventGroup::getFdEventGroup()
    {
        if (g_fd_event_group != nullptr)
        {
            return g_fd_event_group; // 如果已存在单例对象，则直接返回
        }
        g_fd_event_group = new FdEventGroup(128); // 创建新的单例对象
        return g_fd_event_group;
    }
};