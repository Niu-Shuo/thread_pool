#include "status_manager.h"

namespace nrpc
{
    bool StatusManager::register_signals()
    {
        struct sigaction sa;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        sigaddset(&sa.sa_mask, SIGINT);
        sa.sa_handler = StatusManager::_handle_signal;
        int err = sigaction(SIGINT, &sa, nullptr);
        // PLOG_FATAL_IF(err == -1) << "StatusManager failed to invoke sigaction";
        PLOG_FATAL_IF(err == -1) << "StatusManager failed to invoke sigaction"; 
        return true;
    }
    void StatusManager::wait_for_signal(int timeout_ms)
    {
        std::unique_lock<std::mutex> ul(m_mutex);
        // LOG_DEBUG << "StatusManager start to wait " << timeout_ms << " ms";
        m_cv.wait_until(ul, std::chrono::milliseconds(timeout_ms) + std::chrono::system_clock::now());
    }
} // namespace nrpc
