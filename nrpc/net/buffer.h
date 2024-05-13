#ifndef NRPC_NET_BUFFER_H
#define NRPC_NET_BUFFER_H

#include <string.h>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <ostream>

namespace nrpc
{
    /*
        Buffer 类：用于管理缓冲区的读写操作
        1、提供了默认构造函数和带参数的构造函数用于初始化缓冲区，并在析构时释放资源。
        2、状态成员函数：返回可读、可写取数据的字节数，返回读取区域和写入区域的指针，返回缓冲区的指针。
        3、输入流成员函数：从缓冲区中读取 int32 和 uint32 类型的数据。
        4、读取和写入成员函数：从缓冲区中读取指定字节数的数据，用于将数据从内存写入到文件描述符或从文件描述符读取到缓冲区。
        5、追加数据：向缓冲区追加字符串或整数数据，追加额外的数据。
        6、清空缓冲区

        BufferView 类：用于创建缓冲区的视图，提供了访问缓冲区数据的接口。
        重载了 operator<<，用于将 BufferView 中的数据流输出到流对象中
    */
    class Buffer
    {
    public:
        Buffer(size_t buf_size) : m_rindex(0), m_windex(0), m_data(buf_size), m_extra_data(0) {}
        Buffer() : m_rindex(0), m_windex(0), m_data(InitialBufferSize), m_extra_index(0) {}
        virtual ~Buffer() {}

        // 状态成员函数
        // 可读字节数
        size_t readable() const
        {
            return m_windex - m_rindex;
        }

        // 可写字节数
        size_t writeable() const
        {
            return m_data.size() - m_windex;
        }
        // 读取区域指针
        char *begin()
        {
            return &m_data[m_rindex];
        }
        // 写入区域指针
        char *end()
        {
            return &m_data[m_windex];
        }
        // 缓冲区指针
        const char *data() const
        {
            return &m_data[0];
        }

        // 额外数据
        size_t extraable() const
        {
            return m_extra_data.size() - m_extra_index;
        }

        const char *extra_begin() const
        {
            return &m_extra_data[m_extra_index];
        }

        // 输入流成员函数-int32
        int32_t peek_int32()
        {
            int32_t *tmp = reinterpret_cast<int32_t *>(&m_data[m_rindex]);
            int32_t num = ntohl(*tmp);
            return num;
        }
        // 输入流成员函数-uint32
        uint32_t peek_uint32()
        {
            uint32_t *tmp = reinterpret_cast<uint32_t *>(&m_data[m_rindex]);
            uint32_t num = ntohl(*tmp);
            return num;
        }

        // find crlf
        int find_crlf(const size_t &len);

        // 读成员函数
        // 从缓冲区中移除指定数量的字节数据
        void retrieve(size_t n)
        {
            m_rindex += n; // 将 m_rindex 更新为指向未读数据的新位置
        }
        // 将缓冲区中的数据写入到文件描述符（Socket）中
        int write_fd(int fd);

        // 写成员函数
        // 从文件描述符（Socket）中读取数据并写入到缓冲区中
        int read_fd(int fd);

        // 追加 char*
        void append(const char *str)
        {
            _append(str, strlen(str));
        }
        // 追加 std::string
        void append(const std::string &str)
        {
            _append(str.data(), static_cast<int>(str.length()));
        }
        // 追加 int32_t
        void append(int32_t num)
        {
            int32_t numm = htonl(num);
            _append(&numm, static_cast<int>(sizeof(int32_t)));
        }
        // 追加 uint32_t
        void append(uint32_t num)
        {
            uint32_t numm = htonl(num);
            _append(&numm, static_cast<int>(sizeof(uint32_t)));
        }

        // 接受一个常量引用 extra_data，将其内容复制到 m_extra_data 中。这种方式适用于传递常规的 std::string 类型的数据。
        void append_extra(const std::string &extra_data)
        {
            m_extra_data = extra_data;
        }
        // 接受一个右值引用 extra_data，通过 std::move 将其内容移动到 m_extra_data 中，同时清空 extra_data。
        void append_extra(std::string &&extra_data)
        {
            m_extra_data.swap(extra_data); // 隐式调用std::move()
        }

        // 清空
        void clear()
        {
            m_rindex = m_windex = m_extra_index = 0;
        }

    private:
        // 向缓冲区中追加数据
        void _append(const void *mem, int memlen)
        {
            // 检查缓冲区中可写的空间是否足够容纳要追加的数据，如果不够，就需要扩展缓冲区的大小。
            if (static_cast<int>(writeable()) < memlen)
            {
                m_data.resize(m_data.size() * 2);
            }
            // 使用 memcpy 将 mem 中的数据复制到缓冲区中，从当前写入位置 m_windex 开始。
            memcpy(&m_data[m_windex], mem, memlen);
            // 更新写入索引 m_windex，使其指向新追加的数据的末尾。
            m_windex += memlen;
        }

    public:
        static const int InitialBufferSize = 1024; // 初始化缓冲区的大小为1024

    protected:
        size_t m_rindex, m_windex; // 读索引和写索引，用于标识缓冲区中可读和可写的位置
        std::vector<char> m_data;  // 存储实际的数据，是缓冲区的主体
        size_t m_extra_index;      // 标识额外数据的索引，表示额外数据的起始位置
        std::string m_extra_data;  // 存储额外的数据，可能不会直接存储在缓冲区中，而是被存储在这个字符串中
        friend class BufferView;   // 声明了BufferView类是Buffer类的友元类，这意味着BufferView类可以访问Buffer类的私有成员。
    };

    class BufferView
    {
    public:
        BufferView() : m_buf(nullptr), m_begin(0), m_end(0) {}
        // 接受一个Buffer对象和一个长度参数。它将Buffer对象的地址存储在m_buf中，并使用Buffer对象的读索引作为起始位置，根据给定的长度计算结束位置
        BufferView(const Buffer &buf, size_t length) : m_buf(&buf), m_begin(m_buf->m_rindex), m_end(m_begin + length) {}
        // 接受一个Buffer对象和两个索引参数，分别表示起始位置和结束位置
        BufferView(const Buffer &buf, size_t begin, size_t end) : m_buf(&buf), m_begin(begin), m_end(end) {}
        // 接受一个Buffer对象、一个指向缓冲区数据的指针和一个长度参数。它计算起始位置，使得begin指针指向的位置成为缓冲区中的起始位置
        BufferView(const Buffer &buf, const char *begin, size_t length) : m_buf(&buf), m_begin(begin - m_buf->data()), m_end(m_begin + length) {}

        bool empty() const
        {
            return m_begin == m_end;
        }

        size_t size() const
        {
            return m_end - m_begin;
        }
        std::string str() const
        {
            if (!m_buf)
            {
                return std::string();
            }
            return std::string(m_buf->data() + m_begin, m_buf->data() + m_end);
        }

    private:
        const Buffer *m_buf;   // BufferView关联的缓冲区
        size_t m_begin, m_end; // BufferView对象在缓冲区中的起始位置和结束位置
    };

    // 重载 << ，将 BufferView 对象输出到流中
    inline std::ostream &operator<<(std::ostream &os, const BufferView &bufview)
    {
        os << bufview.str();
        return os;
    }

} // namespace nrpc

#endif