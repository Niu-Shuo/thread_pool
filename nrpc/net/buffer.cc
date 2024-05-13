#include "buffer.h"
#include "base/logging.h"
#include <unistd.h>

namespace nrpc
{
    /*
        find_crlf 在读缓冲区中
        -1  : 没找到
        >=0 : 找到了 \r\n
    */
    int Buffer::find_crlf(const size_t &len)
    {
        static const char *CRLF = "\r\n";
        static const size_t CRLFLen = 2;

        // 用 memmem 函数在给定的缓冲区数据中查找 "\r\n"，如果找到则返回指向该位置的指针，否则返回 nullptr。
        void *ret = memmem(static_cast<const void *>(begin()), len, static_cast<const void *>(CRLF), CRLFLen);
        if (ret == nullptr)
        {
            return -1;
        }
        // 返回其在缓冲区中的位置索引
        return static_cast<const char *>(ret) - begin();
    }

    /*
        将写缓冲区的数据写入到套接字中的功能，并返回写入的数据大小
        -1 - EAGAIN
        0  - EPIPE
        >0 - 数据大小
        TODO data 和 extra_data 放到一起
    */
    int Buffer::write_fd(int fd)
    {
        // 初始化总共写入的数据大小
        int total = 0;
        // 检查写缓冲区中是否有可读数据
        if (readable())
        {
            // 使用系统调用 write 将可读数据写入到套接字中，返回写入的数据大小
            int n = ::write(fd, begin(), readable());
            if (n == -1)
            {
                // 如果错误码是 EAGAIN、EWOULDBLOCK 或者 EINTR，表示应用程序可以稍后再试，此时返回 -1
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                {
                    return -1;
                }
                // 如果错误码是 EPIPE，表示套接字的另一端已经关闭，此时返回 0
                else if (errno == EPIPE)
                {
                    return 0;
                }
                // 其他错误码表示写入发生错误，应用程序需要处理
                else
                {
                    PLOG_FATAL << "Buffer failed to invoke ::write";
                }
            }
            // 如果写入成功，更新读索引和累计写入数据大小
            else
            {
                m_rindex += n;
                total += n;
            }
        }
        // 检查额外数据缓冲区中是否有数据可写入套接字
        if (extraable())
        {
            // 使用系统调用 write 将额外数据写入到套接字中，返回写入的数据大小
            int n = ::write(fd, extra_begin(), extraable());
            // 写入失败，处理方式同上
            if (n == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                {
                    return -1;
                }
                else if (errno == EPIPE)
                {
                    return 0;
                }
                else
                {
                    PLOG_FATAL << "Buffer failed to invoke ::write";
                }
            }
            // 更新额外数据缓冲区的写索引和累计写入数据大小
            else
            {
                m_extra_index += n;
                total += n;
            }
        }
        // 返回总共写入的数据大小
        return total;
    }

    /*
        从套接字中读取数据到写缓冲区的功能，并返回读取的数据大小
        -1 : EAGAIN
         0 : EOF
        >0 : 写入数据大小
        TODO EPIPE / ECONNRESET情况的返回值更新
    */
    int Buffer::read_fd(int fd)
    {
        // 检查写缓冲区是否有足够的空间用于写入数据，如果不够，则扩展写缓冲区大小
        if (writeable() == 0)
        {
            m_data.resize(m_data.size() * 2);
        }
        // 系统调用 read 从套接字中读取数据，写入到写缓冲区中，返回读取的数据大小
        int n = ::read(fd, end(), writeable());
        if (n == -1)
        {
            // 如果错误码是 EAGAIN、EWOULDBLOCK 或者 EINTR，表示应用程序可以稍后再试，此时返回 -1
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                return -1;
            }
            // 其他错误码表示读取发生错误，应用程序需要处理
            else
            {
                PLOG_FATAL << "Buffer failed to invoke ::read";
            }
        }
        // 更新写缓冲区的写索引，表示已经写入了多少数据
        else
        {
            m_windex += n;
        }
        return n;
    }
} // namespace nrpc
