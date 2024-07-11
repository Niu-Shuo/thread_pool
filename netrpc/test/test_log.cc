/*
 * @Description: 日志类测试
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-14 14:53:28
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-14 15:15:56
 */
#include "netrpc/common/log.h"
#include "netrpc/common/config.h"
#include <pthread.h>
#include <thread>

void* fun(void*) {
    int ii = 20;
    while (ii--) {
        printf("1111111\n");
        DEBUGLOG("debug this is thread in %s", "fun");
        // INFOLOG("info this is thread in %s", "fun");
    }
    return nullptr;
}

int main() {
    netrpc::Config::GetInst().Init(nullptr);
    netrpc::Config::GetInst().Init(0);

    // pthread_t thread;
    // pthread_create(&thread, NULL, &fun, NULL);
    std::thread t1(&fun, nullptr);

    int ii = 20;
    while (ii--) {
        DEBUGLOG("test debug log %s", "11");
        INFOLOG("test info log %s", "11");
    }

    // pthread_join(thread, NULL);
    t1.join();
    return 0;
}