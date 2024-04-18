#include "task.hpp"
#include <iostream>
#include <ctime>
#include <chrono>


int Task::m_taskNum = 0;

Task::Task() {
    m_next = nullptr;
    this->m_arg = nullptr;
    ++m_taskNum;
}

Task::Task(std::string name) {
    this->m_name = name;
    ++m_taskNum;
}

Task::Task(void* arg) {
    m_next = nullptr;
    this->m_arg = arg;
    ++m_taskNum;
}

void Task::createTask(void (*hander)(void*), void *arg) {
    this->hander = hander;
    this->m_arg = arg;
}

Task::~Task() {
    if(--m_taskNum == 0) {
        // 获取当前时间点
        auto now = std::chrono::system_clock::now();
    
        // 将当前时间点转换为 time_t 类型
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    
        // 将 time_t 类型转换为本地时间的 struct tm 结构体
        struct std::tm* now_tm = std::localtime(&now_c);
        
        std::cout << "所有任务执行完成，完成时间为："
              << (now_tm->tm_year + 1900) << "-" // 年
              << (now_tm->tm_mon + 1) << "-"     // 月
              << now_tm->tm_mday << " "          // 日
              << now_tm->tm_hour << ":"          // 时
              << now_tm->tm_min << ":"           // 分
              << now_tm->tm_sec << std::endl;    // 秒
        free(this->m_arg);
    }
}
