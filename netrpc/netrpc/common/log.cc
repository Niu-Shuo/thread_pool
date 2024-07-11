/*
 * @Description: log.cc日志类实现
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 20:14:06
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-14 20:28:13
 */
#include <sys/time.h>
#include <signal.h>
#include "netrpc/common/log.h"
#include "netrpc/net/eventloop.h"
#include "netrpc/common/util.h"
#include "netrpc/common/run_time.h"

namespace netrpc
{
    /*
        当程序收到无效信号时，调用此函数处理核心转储。函数将记录错误日志，等待异步日志线程结束，
        然后恢复信号默认处理并重新发送信号。
    */
    // static Logger* g_logger = nullptr;
    void CoredumpHandler(int signal_no)
    {
        // 记录错误日志，提示进程收到无效信号即将退出
        ERRORLOG("progress received invalid signal, will exit");

        // 等待异步日志记录器线程结束
        pthread_join(Logger::GetInst().getAsyncLogger()->m_thread, nullptr);
        pthread_join(Logger::GetInst().getAsyncAppLogger()->m_thread, nullptr);

        // 将信号处理恢复为默认处理方式
        signal(signal_no, SIG_DFL);

        // 重新发送信号
        raise(signal_no);
    }

    // 根据传入的日志级别返回相应的字符串，若级别不在已定义范围内，返回 "UNKNOWN"。
    // 将日志级别转换为对应的字符串表示
    std::string LogLevelToString(LogLevel level)
    {
        switch (level)
        {
        case Debug:
            return "DEBUG"; // 调试级别日志
        case Info:
            return "INFO"; // 信息级别日志
        case Error:
            return "ERROR"; // 错误级别日志
        default:
            return "UNKNOWN"; // 未知日志级别
        }
    }

    // 根据传入的日志级别字符串返回相应的枚举值，若字符串不在已定义范围内，返回 Unknown。
    // 将字符串转换为对应的日志级别枚举值
    LogLevel StringToLogLevel(const std::string &log_level)
    {
        if (log_level == "DEBUG")
        {
            return Debug; // 调试级别日志
        }
        else if (log_level == "INFO")
        {
            return Info; // 信息级别日志
        }
        else if (log_level == "ERROR")
        {
            return Error; // 错误级别日志
        }
        else
        {
            return Unknown; // 未知日志级别
        }
    }

    // 初始化异步日志记录器，设置文件名、文件路径及最大文件大小，初始化信号量并创建日志记录线程。
    // 异步日志记录器构造函数
    AsyncLogger::AsyncLogger(const std::string &file_name, const std::string &file_path, int max_size)
        : m_file_name(file_name), m_file_path(file_path), m_max_file_size(max_size)
    {
        // 初始化信号量，初始值为0
        sem_init(&m_sempahore, 0, 0);
        // 创建日志记录线程，并确保创建成功
        assert(pthread_create(&m_thread, nullptr, &AsyncLogger::Loop, this) == 0);

        // 等待线程初始化完成
        sem_wait(&m_sempahore);
    }

    // 设置停止标志位为true，以指示日志记录线程停止运行。
    // 停止异步日志记录器
    void AsyncLogger::stop()
    {
        m_stop_flag = true;
    }

    // 如果日志文件句柄存在，刷新日志文件缓冲区以确保所有日志条目都写入文件。
    // 刷新日志文件缓冲区
    void AsyncLogger::flush()
    {
        if (m_file_handler)
        {
            fflush(m_file_handler);
        }
    }

    // 该函数将传入的日志缓冲区推送到异步日志记录队列中，并通知等待的日志记录线程处理这些日志条目。
    // 将日志缓冲区推送到异步日志记录队列
    void AsyncLogger::pushLogBuffer(std::vector<std::string> &vec)
    {
        // 加锁保护共享资源
        std::lock_guard<std::mutex> lock(m_mutex);

        // 将日志缓冲区推送到日志记录队列
        m_buffer.push(vec);

        // 通知等待的日志记录线程
        m_condvariable.notify_one();

        // 需要唤醒异步日志线程
        // printf("pthread_cond_signal\n");
    }

    // 该函数是异步日志记录线程的主循环，将缓冲区中的日志条目写入日志文件。
    // 当缓冲区为空时，线程将休眠，直到有新的日志数据被推送。
    // 异步日志记录线程循环函数
    void *AsyncLogger::Loop(void *arg)
    {
        // 将传入的参数转换为 AsyncLogger 指针
        auto logger = reinterpret_cast<AsyncLogger *>(arg);

        /*
         * 通过使用信号量，确保了 AsyncLogger 对象的构造已完成，并且 Loop 线程已经启动且准备好执行循环逻辑之前，
         * 构造函数不会返回。这种同步机制可以防止出现在构造函数完成之前 Loop 方法就开始操作对象成员变量
         * （如 m_file_name, m_file_path, m_max_file_size 等）的情况，进而防止潜在的竞态条件。
         */
        sem_post(&logger->m_sempahore);

        while (1)
        {
            std::vector<std::string> tmp;
            {
                // 加锁保护共享资源
                std::unique_lock<std::mutex> lock(logger->m_mutex);

                // 等待条件变量，直到缓冲区有数据
                while (logger->m_buffer.empty())
                {
                    logger->m_condvariable.wait(lock);
                }

                // 将缓冲区中的日志数据取出到临时变量 tmp
                tmp.swap(logger->m_buffer.front());
                logger->m_buffer.pop();

                lock.unlock();
            }

            // 获取当前时间和格式，用于生成日志文件名中的日期部分
            struct timeval now;
            gettimeofday(&now, nullptr);

            struct tm now_time;
            localtime_r(&(now.tv_sec), &now_time);

            const char *format = "%Y%m%d";
            // const char * format = "%Y%m{}%H%M{}";

            char date[32];
            strftime(date, sizeof(date), format, &now_time);

            // 如果当前日期变化，重置日志文件编号并设置重新打开文件标志
            if (std::string(date) != logger->m_date)
            {
                logger->m_no = 0;
                logger->m_reopen_flag = true;
                logger->m_date = std::string(date);
            }
            if (logger->m_file_handler == nullptr)
            {
                logger->m_reopen_flag = true;
            }

            // 构造日志文件名
            std::stringstream ss;
            ss << logger->m_file_path << logger->m_file_name << "_"
               << std::string(date) << "_log.";
            std::string log_file_name = ss.str() + std::to_string(logger->m_no);

            // 如果需要重新打开文件
            if (logger->m_reopen_flag)
            {
                if (logger->m_file_handler)
                {
                    // 先将文件关闭
                    fclose(logger->m_file_handler);
                }
                // 打开新的日志文件
                logger->m_file_handler = fopen(log_file_name.c_str(), "a");
                logger->m_reopen_flag = false; // 重置重新打开文件标志
            }

            // 如果当前日志文件大小超过最大限制，关闭当前文件，创建新的文件
            if (ftell(logger->m_file_handler) > logger->m_max_file_size)
            {
                fclose(logger->m_file_handler);

                log_file_name = ss.str() + std::to_string(logger->m_no++);
                logger->m_file_handler = fopen(log_file_name.c_str(), "a");
                logger->m_reopen_flag = false;
            }

            // 将 tmp 中的日志消息写入文件，同时用 fflush 刷新文件缓冲区，确保所有内容都写入磁盘
            for (auto &ii : tmp)
            {
                if (!ii.empty())
                {
                    fwrite(ii.c_str(), 1, ii.length(), logger->m_file_handler);
                }
            }
            fflush(logger->m_file_handler);

            // 检查停止标志，若为 true 则退出循环，结束线程
            if (logger->m_stop_flag)
            {
                return nullptr;
            }
        }
        return nullptr;
    }

    // 该构造函数用于初始化 Logger 对象。根据传入的日志类型参数，执行相应的初始化逻辑。
    Logger::Logger(LogLevel level, int type /*=1*/) : m_set_level(level), m_type(type)
    {
        if (m_type == 0)
        {
            return; // 如果日志类型为 0，不进行进一步的初始化
        }
        // 根据类型执行其他初始化操作
    }

    // 根据日志类型，将日志消息输出到控制台（客户端）或推送到日志缓冲区（服务端）。
    // 将日志消息推送到日志缓冲区
    void Logger::pushLog(const std::string &msg)
    {
        // m_type = 0 表示客户端，1 表示服务端
        if (m_type == 0)
        {
            // 如果日志类型为客户端，直接打印日志消息到控制台
            printf("%s\n", msg.c_str());
            return;
        }

        // 如果日志类型为服务端，将日志消息推送到日志缓冲区
        std::lock_guard<std::mutex> lock(m_mutex);
        m_buffer.push_back(msg);
    }

    // 将应用日志消息推送到应用日志缓冲区，使用互斥锁保护共享资源，确保线程安全。
    // 将应用日志消息推送到应用日志缓冲区
    void Logger::pushAppLog(const std::string &msg)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_app_buffer.push_back(msg);
    }

    void Logger::log()
    {
    }

    // 同步日志缓冲区到异步日志记录器和应用日志异步记录器
    // 将当前日志缓冲区中的日志消息同步到异步日志记录器和应用日志异步记录器的缓冲区队尾。
    // 使用互斥锁保护共享资源，确保线程安全。
    void Logger::syncLoop()
    {
        // 同步 m_buffer 到 async_logger 的 buffer 队尾
        std::vector<std::string> tmp_vec;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            tmp_vec.swap(m_buffer); // 将 m_buffer 中的日志消息移动到临时向量 tmp_vec
        }

        if (!tmp_vec.empty())
        {
            m_asnyc_logger->pushLogBuffer(tmp_vec); // 将 tmp_vec 中的日志消息推送到异步日志记录器
        }
        tmp_vec.clear();

        // 同步 m_app_buffer 到 app_async_logger 的 buffer 队尾
        std::vector<std::string> tmp_vec2;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            tmp_vec2.swap(m_app_buffer); // 将 m_app_buffer 中的应用日志消息移动到临时向量 tmp_vec2
        }

        if (!tmp_vec2.empty())
        {
            m_asnyc_app_logger->pushLogBuffer(tmp_vec2); // 将 tmp_vec2 中的应用日志消息推送到应用日志异步记录器
        }
    }

    // 刷新日志并停止异步日志记录器
    // 刷新当前日志缓冲区到异步日志记录器和应用日志异步记录器，然后停止这两个异步日志记录器的运行，并刷新它们的日志文件缓冲区。
    void Logger::flush()
    {
        // 同步当前日志缓冲区到异步日志记录器和应用日志异步记录器
        syncLoop();

        // 停止异步日志记录器并刷新日志文件缓冲区
        m_asnyc_logger->stop();
        m_asnyc_logger->flush();
        m_asnyc_app_logger->stop();
        m_asnyc_app_logger->flush();
    }

    // 初始化日志系统
    // 根据配置初始化日志系统，设置全局日志级别、日志类型，并创建异步日志记录器和定时器事件。
    // 注册信号处理函数，处理程序崩溃时的核心转储。
    void Logger::Init(int type)
    {
        // 获取全局日志级别
        LogLevel global_log_level = StringToLogLevel(Config::GetInst().m_log_level);
        printf("Init log level [%s]\n", LogLevelToString(global_log_level).c_str());

        // 设置日志类型和全局日志级别
        m_type = type;
        m_set_level = global_log_level;

        // 如果日志类型为客户端，则直接返回，不进行服务端日志初始化
        if (m_type == 0)
        {
            return;
        }

        // 创建应用日志异步记录器和 RPC 日志异步记录器
        m_asnyc_app_logger = std::make_shared<AsyncLogger>(
            Config::GetInst().m_log_file_name + "_app",
            Config::GetInst().m_log_file_path,
            Config::GetInst().m_log_max_file_size);
        m_asnyc_logger = std::make_shared<AsyncLogger>(
            Config::GetInst().m_log_file_name + "_rpc",
            Config::GetInst().m_log_file_path,
            Config::GetInst().m_log_max_file_size);

        // 创建定时器事件，用于定期同步日志缓冲区到日志文件
        m_timer_event = std::make_shared<TimerEvent>(
            Config::GetInst().m_log_sync_interval,
            true,
            std::bind(&Logger::syncLoop, this));
        EventLoop::GetCurrentEventLoop()->addTimerEvent(m_timer_event);

        // 注册信号处理函数，处理程序崩溃时的核心转储
        signal(SIGSEGV, CoredumpHandler);
        signal(SIGABRT, CoredumpHandler);
        signal(SIGTERM, CoredumpHandler);
        signal(SIGKILL, CoredumpHandler);
        signal(SIGINT, CoredumpHandler);
        signal(SIGSTKFLT, CoredumpHandler);
    }

    // 将日志事件对象转换为格式化的字符串表示
    // 将当前日志事件对象的各个属性（日志级别、时间戳、进程ID、线程ID、消息ID、方法名）格式化成特定格式的字符串表示。
    // eg: [INFO]	[24-03-06 16:01:02.116]	[2526464:2526467]	[99998888]	[makeOrder]
    std::string LogEvent::toString()
    {
        struct timeval now_time;
        gettimeofday(&now_time, nullptr);
        struct tm now_time_t;
        localtime_r(&(now_time.tv_sec), &now_time_t);

        // 格式化时间字符串
        char buf[128];
        strftime(buf, sizeof(buf), "%y-%m-%d %H:%M:%S", &now_time_t);
        std::string time_str(buf);
        int ms = now_time.tv_usec / 1000;
        time_str = time_str + "." + std::to_string(ms);

        // 获取当前进程ID和线程ID
        m_pid = getpid();
        m_thread_id = getThreadId();

        // 构建日志字符串
        std::stringstream ss;
        ss << "[" << LogLevelToString(m_level) << "]\t"    // 日志级别
           << "[" << time_str << "]\t"                     // 时间戳
           << "[" << m_pid << ":" << m_thread_id << "]\t"; // 进程ID和线程ID

        // 获取当前线程处理的请求的 msgid 和方法名
        std::string msgid = RunTime::GetRunTime()->m_msgid;
        std::string method_name = RunTime::GetRunTime()->m_method_name;
        if (!msgid.empty())
        {
            ss << "[" << msgid << "]\t"; // 消息ID
        }
        if (!method_name.empty())
        {
            ss << "[" << method_name << "]\t"; // 方法名
        }

        return ss.str(); // 返回格式化后的日志字符串
    }
};
