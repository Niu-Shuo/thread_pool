#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <queue>
#include <mutex>
#include <functional>
#include <thread>
#include <utility>
#include <future>
#include <vector>

// 构造一个线程安全的队列的模板类
template <typename T>
class SafeQueue{
private:
    std::queue<T> m_queue;  // 队列

    std::mutex m_mutex;     // 访问互斥信号量

public:
    SafeQueue() {}
    SafeQueue(SafeQueue &&other) {}
    ~SafeQueue() {}

    // 队列是否为空
    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex); // 互斥信号变量加锁，防止m_queue被改变

        return m_queue.empty();
    }

    // 队列大小
    int size() {
        std::unique_lock<std::mutex> lock(m_mutex); // 互斥信号变量加锁，防止m_queue被改变

        return m_queue.size();
    }

    // 队列添加元素
    void enqueue(T& t){
        std::unique_lock<std::mutex> lock(m_mutex); // 互斥信号变量加锁，防止m_queue被改变
        m_queue.emplace(t);
    }

    // 队列取出元素
    bool dequeue(T& t) {
        std::unique_lock<std::mutex> lock(m_mutex); // 互斥信号变量加锁，防止m_queue被改变

        if (m_queue.empty())
            return false;
        
        t = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
};


class ThreadPool {
private:
    class ThreadWorker {        // 内置线程工作类
        private:
            int m_id;           // 工作id

            ThreadPool* m_pool; // 所属线程池
        
        public:
            // 构造函数
            ThreadWorker(const int id, ThreadPool* pool) : m_id(id), m_pool(pool) {}

            // 重载()
            void operator() () {
                std::function<void()> func; // 定义基础函数类
                
                bool dequeued;
                while(!m_pool->m_shutdown){
                    // 为线程环境加锁，互访问工作线程的休眠和唤醒
                    std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
                    // std::unique_lock<std::mutex> lock(m_pool->m_conditional_lock);

                    // 如果任务队列为空，阻塞当前线程
                    if(m_pool->m_queue.empty()) {
                        m_pool->m_conditional_lock.wait(lock);  // 等待条件变量通知，开启线程
                    }

                    // 取出任务队列中的元素
                    dequeued = m_pool->m_queue.dequeue(func);
                }

                // 如果成功取出，执行工作函数
                if(dequeued) {
                    func();
                }
            }
    };

    bool m_shutdown;    // 线程池是否关闭
    SafeQueue<std::function<void()>> m_queue;   // 执行函数安全队列，也就是任务队列
    std::vector<std::thread> m_threads;         // 工作线程队列
    std::mutex m_conditional_mutex;             // 线程休眠锁互斥变量
    std::condition_variable m_conditional_lock; // 线程环境锁，可以让线程处于休眠或者唤醒

public:
    // 线程池构造函数
    ThreadPool (const int n_threads = 4)
        : m_threads(std::vector<std::thread>(n_threads)), m_shutdown(false) {}
    ThreadPool (const ThreadPool&) = delete;
    ThreadPool (ThreadPool&&) = delete;
    ThreadPool &operator=(const ThreadPool&) = delete;
    ThreadPool &operator=(ThreadPool&&) = delete;

    // 初始化线程池
    void init() {
        for(int ii = 0; ii != m_threads.size(); ++ii) {
            m_threads.at(ii) = std::thread(ThreadWorker(ii, this)); // 分配工作线程
        }
    }

    // 等待知道线程完成当前任务，并关闭线程池
    void shutdown() {
        m_shutdown = true;
        m_conditional_lock.notify_all();    // 通知唤醒所有工作线程

        for(int ii = 0; ii != m_threads.size(); ++ii){
            if(m_threads.at(ii).joinable()){    // 判断线程是否在等待
                m_threads.at(ii).join();        // 将线程加入等待队列
            }
        }
    }

    // Submit a function to be executed asynchronously by the pool
    template <typename F, typename... Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        // Create a function with bounded parameter ready to execute
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...); // 连接函数和参数定义，特殊函数类型，避免左右值错误

        // Encapsulate it into a shared pointer in order to be able to copy construct
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // Warp packaged task into void function
        std::function<void()> warpper_func = [task_ptr]()
        {
            (*task_ptr)();
        };

        // 队列通用安全封包函数，并压入安全队列
        m_queue.enqueue(warpper_func);

        // 唤醒一个等待中的线程
        m_conditional_lock.notify_one();

        // 返回先前注册的任务指针
        return task_ptr->get_future();
    }
};

#endif