/*
 * @Description: log.h 日志类声明
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 18:03:42
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-14 21:23:20
 */
#ifndef NETRPC_COMMON_LOG_H
#define NETRPC_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include <mutex>
#include <semaphore.h>
#include <condition_variable>
#include <sstream>
#include <cstdio>
#include <stdexcept>
#include <pthread.h>
#include <fmt/core.h>
#include "netrpc/common/config.h"
#include "netrpc/net/timer_event.h"

namespace netrpc
{
    // 可变参模板函数，用于格式化字符串
    template <typename... Args>
    std::string formatString(const char *str, Args &&...args)
    {
        // 获取字符串缓冲区的大小
        int size = snprintf(nullptr, 0, str, args...);
        std::string result;
        if (size > 0)
        {
            result.resize(size);
            // 将可变参数和 str 填入 result
            snprintf(&result[0], size + 1, str, args...);
        }
        return result;
    }

    // 定义日志宏
#define DEBUGLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Debug) \
    { \
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushLog((netrpc::LogEvent(netrpc::LogLevel::Debug)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n"); \
    } \

#define INFOLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Info) \
    { \
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushLog((netrpc::LogEvent(netrpc::LogLevel::Info)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n");\
    } \

#define ERRORLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Error) \
    { \
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushLog((netrpc::LogEvent(netrpc::LogLevel::Error)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n"); \
    } \

#define APPDEBUGLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Debug) \
    { \
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushAppLog((netrpc::LogEvent(netrpc::LogLevel::Debug)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n"); \
    } \

#define APPINFOLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Info) \
    { \
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushAppLog((netrpc::LogEvent(netrpc::LogLevel::Info)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n");\
    } \

#define APPERRORLOG(str, ...) \
    if (netrpc::Logger::GetInst().getLogLevel() <= netrpc::Error) \
    { \
        auto msg1 = netrpc::formatString(str, ##__VA_ARGS__); \
        netrpc::Logger::GetInst().pushAppLog((netrpc::LogEvent(netrpc::LogLevel::Error)).toString() \
        + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + msg1 + "\n"); \
    } \

    // 日志级别枚举类型
    enum LogLevel
    {
        Unknown = 0,
        Debug = 1,
        Info = 2,
        Error = 3
    };

    // 日志级别与字符串之间的转换函数声明
    std::string LogLevelToString(LogLevel level);
    LogLevel StringToLogLevel(const std::string &log_level);

    // 异步日志类
    class AsyncLogger
    {
    public:
        using AsyncLoggerPtr = std::shared_ptr<AsyncLogger>;
        AsyncLogger(const std::string &file_name, const std::string &file_path, int max_size);

        void stop();

        // 刷新到磁盘
        void flush();

        void pushLogBuffer(std::vector<std::string> &vec);

    public:
        static void *Loop(void *arg);

    public:
        pthread_t m_thread;

    private:
        // 日志缓冲区
        std::queue<std::vector<std::string>> m_buffer;

        std::string m_file_name; // 日志输出文件名字
        std::string m_file_path; // 日志输出路径
        int m_max_file_size{0};  // 日志单个文件最大大小，单位为字节

        sem_t m_sempahore;

        std::condition_variable m_condvariable;
        std::mutex m_mutex;

        std::string m_date;         // 当前打印日志的文件日期
        FILE *m_file_handler{nullptr}; // 当前打开的日志文件句柄

        bool m_reopen_flag{false};

        int m_no{0}; // 日志文件序号

        bool m_stop_flag{false};
    };

    // 日志管理类
    class Logger
    {
    public:
        using LoggerPtr = std::shared_ptr<Logger>;
        Logger(LogLevel level, int type = 1);

        // 推送日志消息
        void pushLog(const std::string &msg);

        // 推送应用日志消息
        void pushAppLog(const std::string &msg);

        // 执行日志记录操作
        void log();

        LogLevel getLogLevel() const
        {
            return m_set_level;
        }

        AsyncLogger::AsyncLoggerPtr getAsyncAppLogger()
        {
            return m_asnyc_app_logger;
        }

        AsyncLogger::AsyncLoggerPtr getAsyncLogger()
        {
            return m_asnyc_logger;
        }

        // 同步日志循环
        void syncLoop();

        // 刷新日志
        void flush();

    public:
        // 禁用拷贝构造和赋值操作
        Logger(const Logger &) = delete;
        Logger &operator=(const Logger &) = delete;

        // 获取Logger单例实例
        static Logger &GetInst()
        {
            static Logger inst;
            return inst;
        }

        // 初始化Logger
        void Init(int type = 1);

    private:
        Logger() = default;
        LogLevel m_set_level;                           // 当前日志级别
        std::vector<std::string> m_buffer;              // 日志缓冲区
        std::vector<std::string> m_app_buffer;          // 应用日志缓冲区
        std::mutex m_mutex;                             // 日志缓冲区互斥量
        std::mutex m_app_mutex;                         // 应用日志缓冲区互斥量
        std::string m_file_name;                        // 日志文件名
        std::string m_file_path;                        // 日志文件路径
        int m_max_file_size{0};                         // 日志文件最大大小
        AsyncLogger::AsyncLoggerPtr m_asnyc_logger;     // 异步日志指针
        AsyncLogger::AsyncLoggerPtr m_asnyc_app_logger; // 应用异步日志指针
        TimerEvent::TimerEventPtr m_timer_event;        // 定时事件指针
        int m_type{0};                                  // 日志类型
    };

    // 日志事件类
    class LogEvent
    {
    public:
        LogEvent(LogLevel level) : m_level(level) {}

        std::string getFileName() const
        {
            return m_file_name;
        }

        LogLevel getLogLevel() const
        {
            return m_level;
        }

        // 将日志事件转换为字符串
        std::string toString();

    private:
        std::string m_file_name; // 文件名
        int32_t m_file_line;     // 行号
        int32_t m_pid;           // 进程号
        int32_t m_thread_id;     // 线程号
        LogLevel m_level;        // 日志级别
    };
};

#endif