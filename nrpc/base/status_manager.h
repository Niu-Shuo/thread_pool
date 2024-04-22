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
    class StatusManager{
    public:
        static StatusManager& get_instance() {
            static StatusManager sm;
            return sm;
        }
    private:
        static void _handle_signal(int signo){
            StatusManager& sm = StatusManager::get_instance();
            sm._set_close();
        }
    public:
        // 注册信号
        bool register_signals();

        // 等待信号
        void wait_for_signal(int timeout_ms = 500);

        // 是否关闭
        bool is_close() {
            std::unique_lock<std::mutex> ul(m_mutex);
            return m_exit;
        }
    private:
        // 设置关闭
        void _set_close() {
            std::unique_lock<std::mutex> ul(m_mutex);
            m_exit = true;
            m_cv.notify_all();
        }
    protected:
        StatusManager() : m_exit(false) {}
        StatusManager(const StatusManager& taht) = delete;
        StatusManager& operator=(const StatusManager& that) = delete;
    private:
        bool m_exit;
        std::mutex m_mutex;
        std::condition_variable m_cv;

    };
} // namespace nrpc


#endif