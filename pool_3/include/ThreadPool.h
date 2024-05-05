#ifndef POOL_3_INCLUDE_THREAD_POOL_H
#define POOL_3_INCLUDE_THREAD_POOL_H

#include <functional>
#include <thread>
#include <condition_variable>
#include <future>

#include "SafeQueue.h"

class ThreadPool {
public:
    ThreadPool(const int n_threads) : m_threads(std::vector<std::thread>(n_threads)), m_shutdown(false) {}
    
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;

    ThreadPool & operator=(const ThreadPool &) = delete;
    ThreadPool & operator=(ThreadPool &&) = delete;

    // 初始化资源池
    void init() {
        for (int i = 0; i != m_threads.size(); ++i) {
            m_threads[i] = std::thread(ThreadWorker(i, this));
        }
    }

    // 全部任务完成关闭线程池
    void shutdown() {
        m_shutdown = true;
        m_conditional_lock.notify_all();

        for (int i = 0; i != m_threads.size(); ++i) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }
    }

    // 提交任务线程池异步执行
    template <typename F, typename ...Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        // 创建一个带有有界参数的函数，准备执行
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args...)());
        // 将其封装到shared_ptr 中以便能够复制构造/分配
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...)())>>(func);
        // 将打包的任务包装到void函数中
        std::function<void()> wrapper_func = [task_ptr]() {
            (*task_ptr)();
        };
        // 将通用包装函数入队
        m_queue.enqueue(wrapper_func);
        // 唤醒一个正在等待的线程
        m_conditional_lock.notify_one();
        // 从 Promise 返回 future
        return task_ptr->get_future();
    }
private:
    class ThreadWorker {
    public:
        ThreadWorker(const int id, ThreadPool* pool) : m_id(id), m_pool(pool) {}
        void operator() () {
            std::function<void()> func;
            bool dequeued;
            while (!m_pool->m_shutdown) {
                std::unique_lock<std::mutex> lock(m_pool->m_condition_mutex);
                if (m_pool->m_queue.dequeue(func)) {
                    m_pool->m_conditional_lock.wait(lock);
                }
                dequeued = m_pool->m_queue.dequeue(func);
            }
            if (dequeued) {
                func();
            }
        }
    private:
        int m_id;
        ThreadPool* m_pool;
    };

    bool m_shutdown;
    SafeQueue<std::function<void()>> m_queue;
    std::vector<std::thread> m_threads;
    std::mutex m_condition_mutex;
    std::condition_variable m_conditional_lock;
};

#endif