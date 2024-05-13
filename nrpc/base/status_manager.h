#ifndef NRPC_BASE_STATUS_MANAGER_H
#define NRPC_BASE_STATUS_MANAGER_H

#include <mutex>
#include <condition_variable>
#include <signal.h>
#include <chrono>
#include <errno.h>
#include <string.h>
#include "logging.h"

namespace nrpc
{
    // 状态管理器类，用于管理程序的运行状态
    class StatusManager
    {
    public:
        // 获取 StatusManager 的单例实例
        static StatusManager &get_instance()
        {
            static StatusManager sm;
            return sm;
        }

    private:
        // 处理信号的回调函数
        static void _handle_signal(int signo)
        {
            StatusManager &sm = StatusManager::get_instance();
            sm._set_close(); // 调用私有函数 _set_close() 设置程序关闭状态
        }

    public:
        // 注册信号
        bool register_signals();

        // 等待信号
        void wait_for_signal(int timeout_ms = 500);

        // 是否关闭
        bool is_close()
        {
            std::unique_lock<std::mutex> ul(m_mutex);
            return m_exit;
        }

    private:
        // 设置程序关闭状态，并唤醒所有等待线程
        void _set_close()
        {
            std::unique_lock<std::mutex> ul(m_mutex);
            m_exit = true;
            m_cv.notify_all();
        }

    protected:
        // 构造函数，初始化成员变量
        StatusManager() : m_exit(false) {}
        // 禁止拷贝构造和赋值操作
        StatusManager(const StatusManager &taht) = delete;
        StatusManager &operator=(const StatusManager &that) = delete;

    private:
        bool m_exit;                  // 程序关闭标志
        std::mutex m_mutex;           // 互斥锁，用于保护共享变量
        std::condition_variable m_cv; // 条件变量，用于线程等待和唤醒
    };
} // namespace nrpc

#endif