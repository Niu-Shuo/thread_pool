#include "task.hpp"
#include "thread_pool.hpp"
#include <iostream>

void executeTask(void* size) {
    int* s = static_cast<int*> (size);
    int value = *s;
    long sum = 0;
    for(int ii = 0; ii != value; ++ii) {
        sum += ii;
    }
    std::cout << "计算完成, sum = " << sum << std::endl;
}

int main() {
    long* n = new long;
    *n = 1000000000;

    // 开始时间
    auto start = std::chrono::system_clock::now();
    executeTask(n);
    executeTask(n);
    executeTask(n);
    executeTask(n);

    // 结束时间
    auto end = std::chrono::system_clock::now();
    // 计算时间差
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // 输出时间差（以毫秒为单位）
    std::cout << "一般流程执行所有任务所需时间: " << duration.count() << "毫秒" << std::endl;

    ThreadPool pool(4);
    Task* t1 = new Task("Task1");
    t1->createTask(executeTask, n);
    Task* t2 = new Task("Task2");
    t2->createTask(executeTask, n);
    Task* t3 = new Task("Task3");
    t3->createTask(executeTask, n);
    Task* t4 = new Task("Task4");
    t4->createTask(executeTask, n);

    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    
    // 将当前时间点转换为 time_t 类型
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    
    // 将 time_t 类型转换为本地时间的 struct tm 结构体
    struct std::tm* now_tm = std::localtime(&now_c);
        
    std::cout << "执行任务的开始时间为："
              << (now_tm->tm_year + 1900) << "-" // 年
              << (now_tm->tm_mon + 1) << "-"     // 月
              << now_tm->tm_mday << " "          // 日
              << now_tm->tm_hour << ":"          // 时
              << now_tm->tm_min << ":"           // 分
              << now_tm->tm_sec << std::endl;    // 秒
    pool.taskPost(t1);
    pool.taskPost(t2);
    pool.taskPost(t3);
    pool.taskPost(t4);

    return 0;
}