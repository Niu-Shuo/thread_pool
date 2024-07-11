/*
 * @Description: RunTime.cc 运行时类实现
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 20:16:21
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-03 20:25:14
 */
#include "netrpc/common/run_time.h"

namespace netrpc
{
    // 声明线程局部变量 t_run_time，用于存储每个线程的 RunTime 实例指针
    thread_local RunTime *t_run_time = nullptr;

    // 获取运行时对象的单例实例
    RunTime *netrpc::RunTime::GetRunTime()
    {
        // 如果当前线程已经有了运行时对象，则直接返回该对象指针
        if (t_run_time)
        {
            return t_run_time;
        }
        // 如果当前线程没有运行时对象，则创建一个新的对象并返回其指针
        t_run_time = new RunTime();
        return t_run_time;
    }
};