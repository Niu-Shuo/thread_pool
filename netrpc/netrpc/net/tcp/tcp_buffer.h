/*
 * @Description: TcpBuffer类头文件，用于处理TCP数据的缓冲区
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-07 20:17:45
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 21:16:58
 */
#ifndef NETRPC_NET_TCP_TCPBUFFER_H
#define NETRPC_NET_TCP_TCPBUFFER_H

#include <vector>
#include <memory>

namespace netrpc
{
    class TcpBuffer
    {
    public:
        using TcpBufferPtr = std::shared_ptr<TcpBuffer>;

        // 构造函数，初始化缓冲区大小
        TcpBuffer(int size);

        // 析构函数
        ~TcpBuffer();

        // 返回缓冲区中可读字节数
        int readAble();

        // 返回缓冲区中可写字节数
        int writeAble();

        // 返回当前读索引
        int readIndex();

        // 返回当前写索引
        int writeIndex();

        // 将数据写入缓冲区
        void writeToBuffer(const char *buf, int size);

        // 从缓冲区读取数据
        void readFromBuffer(std::vector<char> &re, int size);

        // 调整缓冲区大小
        void resizeBuffer(int new_size);

        // 调整缓冲区，将未读数据移到缓冲区开始位置
        void adjustBuffer();

        // 移动读索引
        void moveReadIndex(int size);

        // 移动写索引
        void moveWriteIndex(int size);

    public:
        std::vector<char> m_buffer; // 缓冲区
    private:
        int m_read_index{0};  // 读索引
        int m_write_index{0}; // 写索引
        int m_size{0};        // 缓冲区大小
    };
};

#endif