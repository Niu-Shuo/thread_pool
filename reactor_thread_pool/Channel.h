#ifndef REACTOR_THREAD_POOL_CHANNEL_H
#define REACTOR_THREAD_POOL_CHANNEL_H

#include <functional>

using handleFunc = std::function<int(void *)>;
/*
    参数泛型，handleFunc函数名 可以指向普通函数或者类里面的静态函数
    using handleFunc = int(*)(void* );
    定义文件描述符的读写事件，使用枚举   自定义
*/

//  C++11 强类型枚举
enum class FDEvent
{
    TimeOut = 0x01,   // 十进制1，二进制001
    ReadEvent = 0x02, // 十进制2，二进制010
    WriteEvent = 0x04 // 十进制4，二进制100
};

/*
    可调用对象包装器打包：函数指针，可调用对象(可以向函数一样使用)
    最终得到了地址，但是没有调用
*/

// 描述符了文件描述符和读写，和读写回调函数以及参数
class Channel
{
public:
    // 构造函数
    Channel(int fd, FDEvent events, handleFunc readFunc, handleFunc writeFunc, handleFunc destoryFunc, void *arg);

    // 回调函数
    handleFunc readCallback;    // 读回调
    handleFunc writeCallback;   // 写回调
    handleFunc destroyCallback; // 销毁回调

    // 修改fd的写事件(检测 or 不检测)，flag是否为写事件
    void writeEventEnable(bool flag);

    // 判断的时候需要检测文件描述符的写事件
    bool isWriteEventEnable();

    // 修改fd的读事件(检测 or 不检测)，flag是否为读时间
    void readEventEnable(bool flag);

    // 判断的时候需要检测文件描述符的读事件
    bool isReadEventEnable();

    // // 修改fd的销毁事件(检测 or 不检测)，flag是否为读时间
    // void timeoutEventEnable(bool flag);

    // // 判断的时候需要检测文件描述符的读事件
    // bool isTimeoutEventEnable();

    /*
        内联函数，函数调用不需要压栈，直接进行代码的替换，提高程序执行效率，但也需要更多内存，多用于简单

    */

    // 取出是由成员的值，使用此方法保证了安全性，可以直接读取，但不可修改，但地址仍可以修改 
    // 取出事件
    inline int getEvent(){
        return m_events;
    }

    // 取出文件描述符
    inline int getSocket(){
        return m_fd;
    }

    // 取出回调函数参数
    inline const void* getArg(){
        return m_arg;
    }
private:
    int m_fd;     // 文件描述符
    int m_events; // 读/ 写/ 读写
    void *m_arg;  // 回调函数参数
};

#endif