#ifndef THREAD_NRPC_BASE_LOGGING_H
#define THREAD_NRPC_BASE_LOGGING_H

#include <sstream>
#include <memory>
#include <iostream>
#include <ostream>
#include <string.h>
#include "timestamp.h"
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <sstream>
#include <fcntl.h>
#include <errno.h>

namespace nrpc
{
    const int DEBUG = 0;
    const int NOTICE = 1;
    const int WARNING = 2;
    const int FATAL = 3;

    // 借鉴brpc
    class _LogStream : public std::stringbuf, public std::ostream
    {
    public:
        _LogStream() : std::ostream(static_cast<std::stringbuf *>(this)){};
        const char *pbase()
        {
            return std::stringbuf::pbase();
        }

        const char *pptr()
        {
            return std::stringbuf::pptr();
        }
    };

    class _LogAppender
    {
    public:
        virtual void append(const std::shared_ptr<_LogStream> &ls) = 0;
        virtual ~_LogAppender() {}
    };

    class _LogStdoutAppender : public _LogAppender
    {
    public:
        void append(const std::shared_ptr<_LogStream> &ls)
        {
            std::cout << ls->str();
        }

        virtual ~_LogStdoutAppender() {}
    };

    class _LogFileAppender : public _LogAppender
    {
    public:
        _LogFileAppender() : m_f(nullptr), m_file_node(0), m_rollover_s(-1), m_t(TimeStamp::now()) {}

        virtual bool init(const char *logfile, int rollover_min)
        {
            m_filename = logfile;
            m_rollover_s = rollover_min * 60;
            return open();
        }

        virtual ~_LogFileAppender()
        {
            close();
        }

        void append(const std::shared_ptr<_LogStream> &ls)
        {
            do_rollover();
            int len = ls->pptr() - ls->pbase();

            // fwrite是线程安全的
            int ret = fwrite(ls->pbase(), 1, len, m_f);
            fflush(m_f);

            if (ret < len)
            {
                fprintf(stderr, "filed to append\n");
                close();
                open();
            }
        }

        bool do_rollover()
        {
            TimeStamp now = TimeStamp::now();
            if (m_rollover_s > 0 && (now.sec() - m_t.sec()) >= m_rollover_s)
            {
                close();
                std::string bk_filename = m_filename + "." + m_t.str();
                rename(m_filename.c_str(), bk_filename.c_str());
                open();
                m_t = now;
            }
            else
            {
                int err = stat(m_filename.c_str(), &m_file_stat);
                bool reopen = false;
                do
                {
                    if (err == -1)
                    {
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
                    unsigned long inode = m_file_stat.st_ino;
                    if (m_file_node != inode)
                    {
                        reopen = true;
                    }
                } while (false);
                if (reopen)
                {
                    close();
                    open();
                    m_t = now;
                }
            }
            return true;
        }

        bool open()
        {
            m_f = fopen(m_filename.c_str(), "a");
            if (m_f == nullptr)
            {
                fprintf(stderr, "failed to open %s\n", m_filename.c_str());
                return false;
            }
            stat(m_filename.c_str(), &m_file_stat);
            m_file_node = m_file_stat.st_ino;
            return true;
        }

        void close()
        {
            if (m_f)
            {
                fclose(m_f);
                m_f = nullptr;
            }
        }

    private:
        FILE *m_f;
        unsigned long m_file_node;
        int m_rollover_s;
        TimeStamp m_t;
        std::string m_filename;
        struct stat m_file_stat;
    };

    class _Logger
    {
    public:
        _Logger() : m_severity(nrpc::DEBUG), m_appender(std::shared_ptr<_LogAppender>(new _LogStdoutAppender())) {}

        int severity() const
        {
            return m_severity;
        }

        bool set_severity(int severity)
        {
            if (severity >= nrpc::DEBUG && severity <= nrpc::FATAL)
            {
                m_severity = severity;
                return true;
            }
            fprintf(stderr, "failed to set severity to %d\n", severity);
            return false;
        }

        void append(const std::shared_ptr<_LogStream> &ls)
        {
            m_appender->append(ls);
        }

        bool set_appender(const std::shared_ptr<_LogAppender> &la)
        {
            if (la != nullptr)
            {
                m_appender = la;
                return true;
            }

            fprintf(stderr, "failed to set appender to nullptr\n");
            return false;
        }

    private:
        int m_severity;
        std::shared_ptr<_LogAppender> m_appender;
    };

    extern _Logger _logger;

    class _LogRecord
    {
    public:
        _LogRecord(const char *file, int lineno, int severity, bool abort = false) : m_abort(abort), m_stream(std::make_shared<_LogStream>())
        {
            const char *severity_map[] = {
                "DEBUG", "NOTICE", "WARNING", "FATAL"};
            stream() << severity_map[severity] << ": "
                     << TimeStamp::now() << " * "
                     << getpid() << " * "
                     << pthread_self() << " "
                     << "[" << file << ":" << lineno << "]: ";
        }

        _LogStream &stream()
        {
            return *m_stream;
        }
        virtual ~_LogRecord()
        {
            stream() << "\n";
            nrpc::_logger.append(m_stream);
            if (m_abort)
            {
                abort();
            }
        }

    private:
        bool m_abort;
        std::shared_ptr<_LogStream> m_stream;
    };

    class _PLogRecord : public _LogRecord
    {
    public:
        _PLogRecord(const char *file, int lineno, int serverity, bool abort = false) : _LogRecord(file, lineno, serverity, abort) {}
        ~_PLogRecord()
        {
            char buffer[ErrorMsgLen] = {'\0'};
            stream() << " with err_no: " << errno
                     << ", err_msg: " << strerror_r(errno, buffer, ErrorMsgLen) << ".";
        }

    public:
        static const int ErrorMsgLen = 256;
    };

    inline bool set_severity(int severity)
    {
        return _logger.set_severity(severity);
    }

    inline bool init_log(const char *logfile, int severity, int rollover_min = 30)
    {
        if (!set_severity(severity))
        {
            return false;
        }

        std::shared_ptr<_LogFileAppender> appender = std::make_shared<_LogFileAppender>();
        if (appender == nullptr || !appender->init(logfile, rollover_min))
        {
            return false;
        }
        return _logger.set_appender(appender);
    }
} // namespace nrpc

// LOG_*
#define LOG_DEBUG if (nrpc::_logger.severity() <= nrpc::DEBUG) \
    nrpc::_LogRecord(__FILE__, __LINE__, nrpc::DEBUG).stream()

#define LOG_NOTICE if (nrpc::_logger.severity() <= nrpc::NOTICE) \
    nrpc::_LogRecord(__FILE__, __LINE__, nrpc::NOTICE).stream()

#define LOG_WARNING if (nrpc::_logger.severity() <= nrpc::WARNING) \
    nrpc::_LogRecord(__FILE__, __LINE__, nrpc::WARNING).stream()

#define LOG_FATAL if (nrpc::_logger.severity() <= nrpc::FATAL) \
    nrpc::_LogRecord(__FILE__, __LINE__, nrpc::FATAL, true).stream()

// PLOG_*
#define PLOG_DEBUG if (nrpc::_logger.severity() <= nrpc::DEBUG) \
    nrpc::_PLogRecord(__FILE__, __LINE__, nrpc::DEBUG).stream()

#define PLOG_NOTICE if (nrpc::_logger.severity() <= nrpc::NOTICE) \
    nrpc::_PLogRecord(__FILE__, __LINE__, nrpc::NOTICE).stream()

#define PLOG_WARNING if (nrpc::_logger.severity() <= nrpc::WARNING) \
    nrpc::_PLogRecord(__FILE__, __LINE__, nrpc::WARNING).stream()

#define PLOG_FATAL if (nrpc::_logger.severity() <= nrpc::FATAL) \
    nrpc::_PLogRecord(__FILE__, __LINE__, nrpc::FATAL, true).stream()

// PLOG_*_IF
#define PLOG_DEBUG_IF(expr) if (expr) PLOG_DEBUG
#define PLOG_NOTICE_IF(expr) if (expr) PLOG_NOTICE
#define PLOG_WARNING_IF(expr) if (expr) PLOG_WARNING
#define PLOG_FATAL_IF(expr) if (expr) PLOG_FATAL

#endif