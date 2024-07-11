/*
 * @Description: TcpBuffer类实现文件，用于处理TCP数据的缓冲区
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-07 20:20:04
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-07 20:57:54
 */
#include "netrpc/net/tcp/tcp_buffer.h"
#include "netrpc/common/log.h"

namespace netrpc
{

    // 构造函数，初始化缓冲区大小
    TcpBuffer::TcpBuffer(int size) : m_size(size)
    {
        m_buffer.resize(size); // 调整缓冲区大小
    }

    // 析构函数
    TcpBuffer::~TcpBuffer()
    {
        // 无需手动释放资源，vector会自动管理内存
    }

    // 返回缓冲区中可读字节数
    int TcpBuffer::readAble()
    {
        return m_write_index - m_read_index; // 写索引减去读索引得到可读字节数
    }

    // 返回缓冲区中可写字节数
    int TcpBuffer::writeAble()
    {
        return m_buffer.size() - m_write_index; // 缓冲区大小减去写索引得到可写字节数
    }

    // 返回当前读索引
    int TcpBuffer::readIndex()
    {
        return m_read_index;
    }

    // 返回当前写索引（注意这里应该是写索引）
    int TcpBuffer::writeIndex()
    {
        return m_write_index;
    }

    // 将数据写入缓冲区
    void TcpBuffer::writeToBuffer(const char *buf, int size)
    {
        // 如果可写字节数不够，则调整缓冲区大小
        if (size > writeAble())
        {
            // 调整缓冲区大小，扩容
            int new_size = (int)(1.5 * (m_write_index + size));
            resizeBuffer(new_size);
        }
        // 将数据复制到缓冲区
        memcpy(&m_buffer[m_write_index], buf, size);
        m_write_index += size;
    }

    // 从缓冲区读取数据
    void TcpBuffer::readFromBuffer(std::vector<char> &re, int size)
    {
        // 如果没有可读数据，则直接返回
        if (readAble() == 0)
        {
            return;
        }
        // 计算实际读取的字节数
        int read_size = readAble() > size ? size : readAble();
        std::vector<char> tmp(read_size);
        memcpy(&tmp[0], &m_buffer[m_read_index], read_size);

        re.swap(tmp);              // 将读取的数据存入re中
        m_read_index += read_size; // 更新读索引
        adjustBuffer();            // 调整缓冲区
    }

    // 调整缓冲区大小
    void TcpBuffer::resizeBuffer(int new_size)
    {
        std::vector<char> tmp(new_size);
        int cnt = std::min(new_size, readAble());

        // 将可读的数据移到新的缓冲区的头部
        memcpy(&tmp[0], &m_buffer[m_read_index], cnt);
        m_buffer.swap(tmp); // 交换缓冲区

        m_read_index = 0;
        m_write_index = m_read_index + cnt;
    }

    // 调整缓冲区，将未读数据移到缓冲区开始位置
    void TcpBuffer::adjustBuffer()
    {
        // 如果读索引小于缓冲区大小的1/3，则不调整
        if (m_read_index < (int)(m_buffer.size() / 3))
        {
            return;
        }
        std::vector<char> buffer(m_buffer.size());
        int cnt = readAble();
        // 将可读数据移到新缓冲区的头部
        memcpy(&buffer[0], &m_buffer[m_read_index], cnt);
        m_buffer.swap(buffer);
        m_read_index = 0;
        m_write_index = m_read_index + cnt;
        buffer.clear();
    }

    // 移动读索引
    void TcpBuffer::moveReadIndex(int size)
    {
        size_t jj = m_read_index + size;
        // 如果新的读索引超过缓冲区大小，打印错误日志
        if (jj >= m_buffer.size())
        {
            ERRORLOG("moveReadIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_index, m_buffer.size());
            return;
        }
        m_read_index = jj;
        adjustBuffer(); // 调整缓冲区
    }

    // 移动写索引
    void TcpBuffer::moveWriteIndex(int size)
    {
        size_t jj = m_write_index + size;
        // 如果新的写索引超过缓冲区大小，打印错误日志
        if (jj >= m_buffer.size())
        {
            ERRORLOG("moveReadIndex error, invalid size %d, old_read_index %d, buffer size %d", size, m_read_index, m_buffer.size());
            return;
        }
        m_write_index = jj;
        adjustBuffer(); // 调整缓冲区
    }
};