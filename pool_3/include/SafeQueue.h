#ifndef POOL_3_INCLUDE_SAFE_QUEUE_H
#define POOL_3_INCLUDE_SAFE_QUEUE_H

#include <mutex>
#include <queue>

// 使用std::queue实现线程安全队列
template <typename T>
class SafeQueue {
public:
    SafeQueue() {}
    SafeQueue(SafeQueue& other) {
        // TODO
    }
    ~SafeQueue() {}
    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    int size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void enqueue(T& t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        // m_queue.push(t);
        m_queue.emplace(t);
    }

    bool dequeue(T& t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        t = std::move(m_queue.front());
        m_queue.pop();
        return ture;
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
};

#endif