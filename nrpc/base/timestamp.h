#ifndef NRPC_BASE_TIMESTAMP_H
#define NRPC_BASE_TIMESTAMP_H

#include <iostream>
#include <sys/time.h>

namespace nrpc
{
    class TimeStamp
    {
    public:
        TimeStamp() : m_sec(0), m_msec(0) {}
        TimeStamp(long sec, long msec) : m_sec(sec), m_msec(msec) {}
        long sec() const
        {
            return m_sec;
        }

        long msec() const
        {
            return m_msec;
        }

        std::string str() const
        {
            char buffer[16];
            struct tm result;
            localtime_r(static_cast<const time_t *>(&m_sec), &result);

            // 例如：20240422100000
            snprintf(buffer, 16, "%04d%02d%02d_%02d%02d%02d",
                     1900 + result.tm_year, 1 + result.tm_mon, result.tm_mday,
                     result.tm_hour, result.tm_min, result.tm_sec);
            return buffer;
        }

        bool operator==(const TimeStamp &that) const
        {
            return m_sec == that.m_sec && m_msec == that.m_msec;
        }

        bool operator<(const TimeStamp &that) const
        {
            if (m_sec != that.m_sec)
                return m_sec < that.m_sec;
            return m_msec < that.m_msec;
        }

        int operator-(const TimeStamp &that) const
        {
            return static_cast<int>((m_sec - that.m_sec) * 1000 + m_msec - that.m_msec);
        }

    public:
        static TimeStamp now()
        {
            struct timeval tv;
            ::gettimeofday(&tv, nullptr);
            long sec = tv.tv_sec;
            long msec = tv.tv_usec / 1000;
            return TimeStamp(sec, msec);
        }

        static TimeStamp after_now_ms(long after_time_ms)
        {
            TimeStamp ts = now();
            long tmp = ts.m_msec + after_time_ms;
            ts.m_sec += tmp / 1000;
            ts.m_msec = tmp % 1000;
            return ts;
        }

    private:
        long m_sec;  // 秒
        long m_msec; // 毫秒
        friend std::ostream &operator<<(std::ostream &os, const TimeStamp &ts);
    };
    inline std::ostream &operator<<(std::ostream &os, const TimeStamp &ts)
    {
        char buffer[32];
        struct tm result;
        localtime_r(static_cast<const time_t *>(&ts.m_sec), &result);

        // 例如：2024-04-22 10:00:00:000
        snprintf(buffer, 32, "%04d-%02d-%02d %02d:%02d:%02d:%03ld",
                 1900 + result.tm_year, 1 + result.tm_mon, result.tm_mday,
                 result.tm_hour, result.tm_min, result.tm_sec, ts.m_msec);
        os << buffer;
        return os;
    }
} // namespace nrpc

#endif