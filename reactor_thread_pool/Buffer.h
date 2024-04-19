#ifndef REACTOR_THREAD_POOL_BUFFER_HPP
#define REACTOR_THREAD_POOL_BUFFER_HPP

#include <string>

// 读写内存类，添加字符串，接受套接字数据，将写缓存区数据发送
class Buffer
{
public:
    // 初始化Buffer，size为申请大小,可根据Buffer读写指针进行相关操作
    Buffer(int size);

    // 析构，释放内存
    ~Buffer();

    // Buffer扩容函数,size实际需要的大小
    void extendRoom(int size);

    // 得到剩余可写的内存容量
    inline int writeableSize()
    {
        return m_capacity - m_readPos;
    }

    // 得到剩余可读的内存容量
    inline int readableSize()
    {
        return m_wirtePos - m_readPos;
    }

    /*
        写内存，两种方式
        1、直接写
        2、通过套接字
    */
    // 1、直接写
    // 将data所指向的数据存到buffer，size表示大小
    int appendString(const char *data, int size);

    // 添加字符串类型
    int appendString(const char *data);
    int appendString(std::string data);

    // 2、套接字
    // 接收套接字，返回接收的数据大小
    int socketRead(int fd);

    // 找到其在数据块中的位置，返回改地址根据 空格换行 取出一行  根据\r\n
    char *findCRLF();

    // 发送数据 缓存区和文件描述符
    int sendData(int socket);

    // 使用Linux的sendfile发送文件
    int sendData(int cfd, int fd, off_t offset, int size);

    // 得到读数据的起始位置
    inline char *data()
    {
        return m_data + m_readPos;
    }

    // 读入数据之后，移动readPos、
    inline int readPosIncrease(int size)
    {
        m_readPos += size;
        return m_readPos;
    }

private:
    char *m_data;       // 指向内存的指针
    int m_capacity;     // 内存容量
    int m_readPos = 0;  // 读地址，记录从什么位置开始读数据
    int m_wirtePos = 0; // 写地址，记录从什么位置开始写数据
};

#endif