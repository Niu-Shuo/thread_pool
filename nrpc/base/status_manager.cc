#include "status_manager.h"

namespace nrpc
{
    // 注册信号处理函数
    bool StatusManager::register_signals()
    {
        struct sigaction sa;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        sigaddset(&sa.sa_mask, SIGINT);
        sa.sa_handler = StatusManager::_handle_signal; // 设置信号处理函数为静态成员函数 _handle_signal
        int err = sigaction(SIGINT, &sa, nullptr);     // 注册 SIGINT 信号的处理函数
        // 如果注册失败，输出致命错误日志
        PLOG_FATAL_IF(err == -1) << "StatusManager failed to invoke sigaction";
        return true;
    }

    // 等待信号
    void StatusManager::wait_for_signal(int timeout_ms)
    {
        std::unique_lock<std::mutex> ul(m_mutex); // 获取互斥锁
        // LOG_DEBUG << "StatusManager start to wait " << timeout_ms << " ms";
        // 等待信号，直到超时或者被唤醒
        m_cv.wait_until(ul, std::chrono::milliseconds(timeout_ms) + std::chrono::system_clock::now());
    }
} // namespace nrpc