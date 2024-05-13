#ifndef NRPC_BASE_TIMESTAMP_H
#define NRPC_BASE_TIMESTAMP_H

#include <iostream>
#include <sys/time.h>

namespace nrpc
{
    class TimeStamp
    {
    public:
        // 默认构造函数，初始化时间戳为0
        TimeStamp() : m_sec(0), m_msec(0) {}

        // 构造函数，根据给定的秒数和毫秒数初始化时间戳
        TimeStamp(long sec, long msec) : m_sec(sec), m_msec(msec) {}

        // 获取秒数
        long sec() const
        {
            return m_sec;
        }

        // 获取毫秒数
        long msec() const
        {
            return m_msec;
        }

        // 返回格式化的时间戳字符串，格式为：YYYYMMDD_HHMMSS
        std::string str() const
        {
            char buffer[16];
            struct tm result;
            localtime_r(static_cast<const time_t *>(&m_sec), &result);
            snprintf(buffer, 16, "%04d%02d%02d_%02d%02d%02d",
                     1900 + result.tm_year, 1 + result.tm_mon, result.tm_mday,
                     result.tm_hour, result.tm_min, result.tm_sec);
            return buffer;
        }

        // 比较两个时间戳是否相等
        bool operator==(const TimeStamp &that) const
        {
            return m_sec == that.m_sec && m_msec == that.m_msec;
        }

        // 比较两个时间戳是否小于
        bool operator<(const TimeStamp &that) const
        {
            if (m_sec != that.m_sec)
                return m_sec < that.m_sec;
            return m_msec < that.m_msec;
        }

        // 计算两个时间戳之间的时间差，单位为毫秒
        int operator-(const TimeStamp &that) const
        {
            return static_cast<int>((m_sec - that.m_sec) * 1000 + m_msec - that.m_msec);
        }

        // 获取当前时间戳
        static TimeStamp now()
        {
            struct timeval tv;
            ::gettimeofday(&tv, nullptr);
            long sec = tv.tv_sec;
            long msec = tv.tv_usec / 1000;
            return TimeStamp(sec, msec);
        }

        // 获取指定毫秒数之后的时间戳
        static TimeStamp after_now_ms(long after_time_ms)
        {
            TimeStamp ts = now();
            long tmp = ts.m_msec + after_time_ms;
            ts.m_sec += tmp / 1000;
            ts.m_msec = tmp % 1000;
            return ts;
        }

    private:
        long m_sec;                                                             // 秒
        long m_msec;                                                            // 毫秒
        friend std::ostream &operator<<(std::ostream &os, const TimeStamp &ts); // 重载输出流操作符，用于将时间戳输出到流中
    };

    // 重载输出流操作符，用于将时间戳输出到流中
    inline std::ostream &operator<<(std::ostream &os, const TimeStamp &ts)
    {
        char buffer[32];
        struct tm result;
        localtime_r(static_cast<const time_t *>(&ts.m_sec), &result);
        snprintf(buffer, 32, "%04d-%02d-%02d %02d:%02d:%02d:%03ld",
                 1900 + result.tm_year, 1 + result.tm_mon, result.tm_mday,
                 result.tm_hour, result.tm_min, result.tm_sec, ts.m_msec);
        os << buffer;
        return os;
    }
} // namespace nrpc

#endif