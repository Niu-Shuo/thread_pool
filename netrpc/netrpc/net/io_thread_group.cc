/*
 * @Description: IOThreadGroup实现文件
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-07 17:25:35
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-07 17:45:01
 */
#include "netrpc/net/io_thread_group.h"
#include "netrpc/common/util.h"

namespace netrpc {

    // 构造函数，根据系统负载动态调整IO线程组的大小
    IOThreadGroup::IOThreadGroup(int size) : m_size(size) {
        // 获取当前CPU使用率和系统平均负载
        double CPU_usage = getCPUUtilization();
        double sys_load = getAverageLoad();

        // 根据系统负载调整IO线程组大小
        if (CPU_usage > 0.8 || sys_load > 0.8) {
            DEBUGLOG("CPU使用率或系统负载较高，需要增加IO线程组大小");
            size = size * 2; // 增加IO线程组大小
        }
        if (CPU_usage < 0.2 || sys_load < 0.2) {
            DEBUGLOG("CPU使用率或系统负载较低，需要减少IO线程组大小");
            size = size / 2; // 减少IO线程组大小
            if (size < 2) {
                size = 2; // 最小线程组大小为2
            }
        }

        // 调整后的大小赋值给成员变量
        m_io_thread_groups.resize(size);

        // 创建IO线程并添加到线程组中
        for (size_t ii = 0; (int)ii < size; ++ii) {
            m_io_thread_groups[ii] = new IOThread();
        }
    }

    // 析构函数，清理资源
    IOThreadGroup::~IOThreadGroup() {
        // 循环删除线程组中的所有IO线程对象
        for (auto& thread : m_io_thread_groups) {
            delete thread;
        }
    }

    // 启动所有IO线程
    void IOThreadGroup::start() {
        for (auto& thread : m_io_thread_groups) {
            thread->start();
        }
    }

    // 等待所有IO线程结束
    void IOThreadGroup::join() {
        for (auto& thread : m_io_thread_groups) {
            thread->join();
        }
    }

    // 获取一个IO线程
    IOThread* IOThreadGroup::getIOThread() {
        if (m_index == (int)m_io_thread_groups.size() || m_index == -1) {
            m_index = 0;
        }
        return m_io_thread_groups[m_index++];
    }
};