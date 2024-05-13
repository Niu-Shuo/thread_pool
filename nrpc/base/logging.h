#ifndef THREAD_NRPC_BASE_LOGGING_H
#define THREAD_NRPC_BASE_LOGGING_H

#include <sstream>
#include <memory>
#include <iostream>
#include <ostream>
#include <string.h>
#include "timestamp.h" // 包含时间戳类的头文件
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <sstream>
#include <fcntl.h>
#include <errno.h>

/*
    日志类
    1、日志级别定义： 定义了 DEBUG、NOTICE、WARNING、FATAL 四个日志级别。
    2、日志输出流类 _LogStream： 该类继承自 std::stringbuf 和 std::ostream，用于管理日志消息的输出。
    3、日志追加器基类 _LogAppender： 定义了一个虚函数 append，用于将日志消息追加到不同的输出目标，如控制台、文件等。
    4、标准输出日志追加器 _LogStdoutAppender： 继承自 _LogAppender，实现了将日志消息输出到标准输出流（std::cout）的功能。
    5、文件日志追加器 _LogFileAppender： 继承自 _LogAppender，实现了将日志消息输出到文件的功能，并支持日志文件滚动（按时间间隔或文件大小）。
    6、日志器类 _Logger： 维护日志级别和日志追加器，提供设置日志级别、追加日志消息等功能。
    7、日志记录类 _LogRecord： 在构造函数中记录日志信息，析构函数中将日志消息追加到日志器中，并根据需要执行异常退出。
    8、带错误信息的日志记录类 _PLogRecord： 继承自 _LogRecord，在析构函数中输出额外的错误信息。
    9、全局日志器对象 _logger： 实例化了一个全局的 _Logger 对象，用于管理日志记录。
    10、宏定义 LOG_* 和 PLOG_*： 定义了一系列宏用于方便地记录不同级别的日志消息，支持条件日志输出。
    11、初始化日志功能 init_log 和设置日志级别功能 set_severity： 提供了初始化日志系统和设置日志级别的接口函数。
*/

namespace nrpc
{
    // 日志级别定义
    const int DEBUG = 0;
    const int NOTICE = 1;
    const int WARNING = 2;
    const int FATAL = 3;

    // 借鉴brpc，定义一个日志输出流类，继承自stringbuf和ostream
    // 前者用于管理字符序列缓冲区，可以将字符序列存储在内存中，后者输出流类，用于向流中写入字符
    class _LogStream : public std::stringbuf, public std::ostream
    {
    public:
        _LogStream() : std::ostream(static_cast<std::stringbuf *>(this)){};
        // 返回指向字符缓冲区起始位置的指针
        const char *pbase()
        {
            return std::stringbuf::pbase();
        }
        // 返回指向当前位置的指针，即指向已写入字符的末尾后面的位置
        const char *pptr()
        {
            return std::stringbuf::pptr();
        }
    };

    // 日志追加器基类
    class _LogAppender
    {
    public:
        virtual void append(const std::shared_ptr<_LogStream> &ls) = 0;
        virtual ~_LogAppender() {}
    };

    // 标准输出日志追加器
    class _LogStdoutAppender : public _LogAppender
    {
    public:
        // 实现了将日志消息追加到标准输出流 std::cout 中
        void append(const std::shared_ptr<_LogStream> &ls)
        {
            std::cout << ls->str();
        }

        virtual ~_LogStdoutAppender() {}
    };

    // 文件日志追加器
    class _LogFileAppender : public _LogAppender
    {
    public:
        _LogFileAppender() : m_f(nullptr), m_file_node(0), m_rollover_s(-1), m_t(TimeStamp::now()) {}
        // 初始化函数，用于设置日志文件和滚动时间
        virtual bool init(const char *logfile, int rollover_min)
        {
            // 设置日志文件名和滚动时间
            m_filename = logfile;
            m_rollover_s = rollover_min * 60;
            // 打开日志文件
            return open();
        }

        virtual ~_LogFileAppender()
        {
            close();
        }

        // 日志追加函数，将日志消息追加到文件中
        void append(const std::shared_ptr<_LogStream> &ls)
        {
            do_rollover();                      // 检查是否需要滚动日志文件
            int len = ls->pptr() - ls->pbase(); // 计算日志消息长度

            // fwrite是线程安全的
            int ret = fwrite(ls->pbase(), 1, len, m_f); // 写入日志消息到文件中
            fflush(m_f);                                // 刷新文件流

            // 检查是否写入成功
            if (ret < len)
            {
                // 输出错误信息并尝试重新打开文件
                fprintf(stderr, "filed to append\n");
                close();
                open();
            }
        }

        // 执行日志文件滚动操作
        bool do_rollover()
        {
            TimeStamp now = TimeStamp::now(); // 获取当前时间
            if (m_rollover_s > 0 && (now.sec() - m_t.sec()) >= m_rollover_s)
            {
                close();                                                // 关闭当前文件
                std::string bk_filename = m_filename + "." + m_t.str(); // 生成备份文件名
                rename(m_filename.c_str(), bk_filename.c_str());        // 重命名当前日志文件为备份文件
                open();                                                 // 重新打开日志文件
                m_t = now;                                              // 更新时间戳
            }
            // 检查日志文件是否发生变化，如果发生则重新打开文件
            else
            {
                int err = stat(m_filename.c_str(), &m_file_stat);
                bool reopen = false;
                do
                {
                    if (err == -1)
                    {
                        // 文件不存在，需要重新打开
                        if (errno == ENOENT)
                        {
                            reopen = true;
                            fprintf(stderr, "do_rollover will reopen. stat err: %d, errmo: %d\n", err, errno);
                        }
                        else
                        {
                            fprintf(stderr, "do_rollover failed to reopen. stat err: %d, errno: %d\n", err, errno);
                        }
                        break;
                    }
                    // 检查文件节点是否发生变化
                    unsigned long inode = m_file_stat.st_ino;
                    if (m_file_node != inode)
                    {
                        reopen = true;
                    }
                } while (false);
                // 如果需要重新打开文件，则执行重新打开操作并更新时间戳
                if (reopen)
                {
                    close();
                    open();
                    m_t = now;
                }
            }
            return true;
        }

        // 打开日志文件
        bool open()
        {
            m_f = fopen(m_filename.c_str(), "a");
            if (m_f == nullptr)
            {
                fprintf(stderr, "failed to open %s\n", m_filename.c_str());
                return false;
            }
            // 获取文件节点号并保存
            stat(m_filename.c_str(), &m_file_stat);
            m_file_node = m_file_stat.st_ino;
            return true;
        }

        // 关闭日志文件
        void close()
        {
            if (m_f)
            {
                fclose(m_f);
                m_f = nullptr;
            }
        }

    private:
        FILE *m_f;                 // 文件指针
        unsigned long m_file_node; // 文件节点号
        int m_rollover_s;          // 滚动时间间隔（单位s）
        TimeStamp m_t;             // 上次滚动时间戳
        std::string m_filename;    // 日志文件名
        struct stat m_file_stat;   // 文件状态信息
    };

    // 日志器类
    class _Logger
    {
    public:
        // 默认构造函数，初始化日志级别和日志追加器为标准输出日志追加器
        _Logger() : m_severity(nrpc::DEBUG), m_appender(std::shared_ptr<_LogAppender>(new _LogStdoutAppender())) {}

        // 获取日志级别
        int severity() const
        {
            return m_severity;
        }

        // 设置日志级别
        bool set_severity(int severity)
        {
            // 检查日志级别是否有效
            if (severity >= nrpc::DEBUG && severity <= nrpc::FATAL)
            {
                m_severity = severity;
                return true;
            }
            // 输出错误信息
            fprintf(stderr, "failed to set severity to %d\n", severity);
            return false;
        }

        // 追加日志
        void append(const std::shared_ptr<_LogStream> &ls)
        {
            // 调用日志追加器的追加函数，将日志消息追加到相应的目标中
            m_appender->append(ls);
        }

        // 设置日志追加器
        bool set_appender(const std::shared_ptr<_LogAppender> &la)
        {
            // 检查日志追加器是否有效
            if (la != nullptr)
            {
                m_appender = la;
                return true;
            }

            // 输出错误信息
            fprintf(stderr, "failed to set appender to nullptr\n");
            return false;
        }

    private:
        int m_severity;                           // 日志器级别
        std::shared_ptr<_LogAppender> m_appender; // 日志追加器
    };

    // 全局日志器对象
    extern _Logger _logger;

    // 日志记录类
    class _LogRecord
    {
    public:
        // 构造函数，初始化日志记录信息和消息流对象
        _LogRecord(const char *file, int lineno, int severity, bool abort = false) : m_abort(abort), m_stream(std::make_shared<_LogStream>())
        {
            // 日志级别映射表
            const char *severity_map[] = {
                "DEBUG", "NOTICE", "WARNING", "FATAL"};
            // 写入日志级别、时间戳、进程ID、线程ID、文件名和行号等信息到消息流中
            stream() << severity_map[severity] << ": "
                     << TimeStamp::now() << " * "
                     << getpid() << " * "
                     << pthread_self() << " "
                     << "[" << file << ":" << lineno << "]: ";
        }

        // 获取消息流对象的引用
        _LogStream &stream()
        {
            return *m_stream;
        }
        // 虚析构函数，用于输出日志消息和处理异常情况
        virtual ~_LogRecord()
        {
            stream() << "\n";               // 在消息流中添加换行符
            nrpc::_logger.append(m_stream); // 将消息流中的日志记录追加到日志器中
            if (m_abort)                    // 如果需要中止程序，则调用abort()函数
            {
                abort();
            }
        }

    private:
        bool m_abort;                         // 是否需要中止程序
        std::shared_ptr<_LogStream> m_stream; // 消息流对象，用于存储日志记录信息
    };

    // 带错误信息的日志记录类
    class _PLogRecord : public _LogRecord
    {
    public:
        // 调用基类的构造函数进行初始化
        _PLogRecord(const char *file, int lineno, int serverity, bool abort = false) : _LogRecord(file, lineno, serverity, abort) {}
        // 用于在日志记录后追加错误信息
        ~_PLogRecord()
        {
            // 定义一个字符数组用于存储错误消息
            char buffer[ErrorMsgLen] = {'\0'};
            // 将错误号和对应的错误消息写入日志消息流中
            stream() << " with err_no: " << errno
                     << ", err_msg: " << strerror_r(errno, buffer, ErrorMsgLen) << ".";
        }

    public:
        static const int ErrorMsgLen = 256; // 错误消息的长度
    };

    // 设置日志级别
    inline bool set_severity(int severity)
    {
        return _logger.set_severity(severity);
    }

    // 初始化日志
    inline bool init_log(const char *logfile, int severity, int rollover_min = 30)
    {
        // 设置日志级别，如果设置失败则返回false
        if (!set_severity(severity))
        {
            return false;
        }

        // 创建文件日志追加器并初始化，如果初始化失败则返回false
        std::shared_ptr<_LogFileAppender> appender = std::make_shared<_LogFileAppender>();
        if (appender == nullptr || !appender->init(logfile, rollover_min))
        {
            return false;
        }
        // 将文件日志追加器设置为日志器的追加器
        return _logger.set_appender(appender);
    }
} // namespace nrpc

// LOG_*
#define LOG_DEBUG                                \
    if (nrpc::_logger.severity() <= nrpc::DEBUG) \
    nrpc::_LogRecord(__FILE__, __LINE__, nrpc::DEBUG).stream()

#define LOG_NOTICE                                \
    if (nrpc::_logger.severity() <= nrpc::NOTICE) \
    nrpc::_LogRecord(__FILE__, __LINE__, nrpc::NOTICE).stream()

#define LOG_WARNING                                \
    if (nrpc::_logger.severity() <= nrpc::WARNING) \
    nrpc::_LogRecord(__FILE__, __LINE__, nrpc::WARNING).stream()

#define LOG_FATAL                                \
    if (nrpc::_logger.severity() <= nrpc::FATAL) \
    nrpc::_LogRecord(__FILE__, __LINE__, nrpc::FATAL, true).stream()

// PLOG_*
#define PLOG_DEBUG                               \
    if (nrpc::_logger.severity() <= nrpc::DEBUG) \
    nrpc::_PLogRecord(__FILE__, __LINE__, nrpc::DEBUG).stream()

#define PLOG_NOTICE                               \
    if (nrpc::_logger.severity() <= nrpc::NOTICE) \
    nrpc::_PLogRecord(__FILE__, __LINE__, nrpc::NOTICE).stream()

#define PLOG_WARNING                               \
    if (nrpc::_logger.severity() <= nrpc::WARNING) \
    nrpc::_PLogRecord(__FILE__, __LINE__, nrpc::WARNING).stream()

#define PLOG_FATAL                               \
    if (nrpc::_logger.severity() <= nrpc::FATAL) \
    nrpc::_PLogRecord(__FILE__, __LINE__, nrpc::FATAL, true).stream()

// PLOG_*_IF
#define PLOG_DEBUG_IF(expr) \
    if (expr)               \
    PLOG_DEBUG
#define PLOG_NOTICE_IF(expr) \
    if (expr)                \
    PLOG_NOTICE
#define PLOG_WARNING_IF(expr) \
    if (expr)                 \
    PLOG_WARNING
#define PLOG_FATAL_IF(expr) \
    if (expr)               \
    PLOG_FATAL

#endif