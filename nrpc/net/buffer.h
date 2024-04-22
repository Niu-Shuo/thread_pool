#ifndef NRPC_BASE_BUFFER_H
#define NRPC_BASE_BUFFER_H

#include <string.h>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <ostream>

namespace nrpc
{
    class Buffer
    {
    public:
        Buffer(size_t buf_size) : m_rindex(0), m_windex(0), m_data(buf_size), m_extra_data(0) {}
        Buffer() : m_rindex(0), m_windex(0), m_data(InitialBufferSize), m_extra_index(0) {}
        virtual ~Buffer() {}

        // 状态成员函数
        // 可读
        inline size_t readable() const
        {
            return m_windex - m_rindex;
        }

        // 可写
        inline size_t writeable() const
        {
            return m_data.size() - m_windex;
        }

        inline char *begin()
        {
            return &m_data[m_rindex];
        }

        inline char *end()
        {
            return &m_data[m_windex];
        }

        inline const char *data() const
        {
            return &m_data[0];
        }

        // 额外数据
        inline size_t extraable() const
        {
            return m_extra_data.size() - m_extra_index;
        }

        inline const char *extra_begin() const
        {
            return &m_extra_data[m_extra_index];
        }

        // 输入流成员函数
        int32_t peek_int32()
        {
            int32_t *tmp = reinterpret_cast<int32_t *>(&m_data[m_rindex]);
            int32_t num = ntohl(*tmp);
            return num;
        }

        uint32_t peek_uint32()
        {
            uint32_t *tmp = reinterpret_cast<uint32_t *>(&m_data[m_rindex]);
            uint32_t num = ntohl(*tmp);
            return num;
        }

        // find crlf
        int find_crlf(const size_t &len);

        // read member functions
        // retrieve data
        inline void retrieve(size_t n)
        {
            m_rindex += n;
        }
        // mem => socket
        int write_fd(int fd);

        // write member functions
        // socket => buffer
        int read_fd(int fd);

        // append
        inline void append(const char *str)
        {
            _append(str, strlen(str));
        }

        inline void append(const std::string &str)
        {
            _append(str.data(), static_cast<int>(str.length()));
        }

        void append(int32_t num)
        {
            int32_t numm = htonl(num);
            _append(&numm, static_cast<int>(sizeof(int32_t)));
        }

        void append(uint32_t num)
        {
            uint32_t numm = htonl(num);
            _append(&numm, static_cast<int>(sizeof(uint32_t)));
        }

        // append_extra
        inline void append_extra(const std::string &extra_data)
        {
            m_extra_data = extra_data;
        }

        inline void append_extra(std::string &&extra_data)
        {
            m_extra_data.swap(extra_data);
        }

        // clear
        inline void clear()
        {
            m_rindex = m_windex = m_extra_index = 0;
        }

    private:
        void _append(const void *mem, int memlen)
        {
            if (static_cast<int>(writeable()) < memlen)
            {
                m_data.resize(m_data.size() * 2);
            }
            memcpy(&m_data[m_windex], mem, memlen);
            m_windex += memlen;
        }

    public:
        static const int InitialBufferSize = 1024;

    protected:
        size_t m_rindex, m_windex;
        std::vector<char> m_data;
        size_t m_extra_index;
        std::string m_extra_data;
        friend class BufferView;
    };

    class BufferView
    {
    };
} // namespace nrpc

#endif