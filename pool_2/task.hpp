#ifndef TASK_POOL_HPP
#define TASK_POOL_HPP
#include <string>


class Task {
public:
    Task();
    Task(std::string name);
    Task(void* arg);
    void createTask(void (*hander)(void* arg), void* arg);
    ~Task();
    void (*hander)(void* arg);
    std::string m_name;
    void* m_arg;
    Task* m_next;  
    static int m_taskNum;
};

#endif