#include "Buffer.h"
#include <strings.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/sendfile.h>

// 构造函数，初始化
Buffer::Buffer(int size) : m_capacity(size)
{
    m_data = (char *)malloc(size);
    bzero(m_data, size);
}

// 析构函数
Buffer::~Buffer()
{
    if (m_data != nullptr)
    {
        free(m_data);
    }
}

void Buffer::extendRoom(int size)
{
    // 1、可写内存够用，不需要扩容
    if (writeableSize() >= size)
    {
        return;
    }

    // 2、合并内存后够用，不需要扩容
    else if (m_readPos + writeableSize() >= size)
    {
        // 得到未读内存的大小
        int readable = readableSize();

        // 移动内存，移动位置，移动的数据地址，数据块大小
        memcpy(m_data, m_data + m_readPos, readable);

        // 更新位置
        m_readPos = 0;
        m_wirtePos = readable;
    }

    // 3、内存不够，扩容
    else
    {
        void *temp = realloc(m_data, m_capacity + static_cast<size_t>(size));

        if (temp == nullptr)
        {
            return;
        }

        // 对扩展的内存进行初始化，temp+buffer->capacity
        memset((char *)temp + m_capacity, 0, size);

        // 更新数据
        m_data = static_cast<char *>(temp);
        m_capacity += size;
    }
}

int Buffer::appendString(const char *data, int size)
{
    // 判断buffer和data是不是指向有效内存
    if (data == nullptr || size == 0)
    {
        return -1;
    }

    // 判断扩容
    extendRoom(size);

    // 数据拷贝，buffer->data + buffer->writePos可写的区域
    memcpy(m_data + m_wirtePos, data, size);
    m_wirtePos += size;
    return 0;
}

int Buffer::appendString(const char *data)
{
    int size = strlen(data);
    int ret = appendString(data, size);
    return ret;
}

int Buffer::appendString(std::string data)
{
    int ret = appendString(data.data());
    return ret;
}

int Buffer::socketRead(int fd)
{
    // read/recv/readv - 接受套接字数据
    // read 把数据存入到一个数组
    // readv 会把数据存入到多个数组，效率更高一些

    struct iovec vec[2];
    int writeable = writeableSize();

    // 初始化数组元素
    vec[0].iov_base = m_data + m_wirtePos;
    vec[0].iov_len = writeable;

    char *tmpbuf = (char *)malloc(40960); // 40k
    vec[1].iov_base = m_data + m_wirtePos;
    vec[1].iov_len = 40960;

    // 接收数据
    int result = readv(fd, vec, 2);
    if (result == -1)
    {
        return -1;
    }
    else if (result <= writeable)
    {
        m_wirtePos += result;
    }
    else
    {
        m_wirtePos = m_capacity; // buffer被写满才会进入这个
        appendString(tmpbuf, result - writeable);
    }
    free(tmpbuf);
    return result;
}

char *Buffer::findCRLF()
{
    // strstr --> 大字符串中匹配子字符串(遇到\0结束)
    // 从haystack开始，寻找needle,从左往右开始寻找
    // char *strstr(const char *haystack, const char *needle);
    // memmem --> 大数据块中匹配子数据块(需要指定数据块大小)
    // void *memmem(const void *haystack, size_t haystacklen,
    // const void* needle, size_t needlelen);
    char *ptr = static_cast<char *>(memmem(m_data + m_readPos, readableSize(), "\r\n", 2));
    return ptr;
}

int Buffer::sendData(int socket)
{
    // 判断buffer里面是否有需要发送的数据,得到未读数据即待发送
    int readable = readableSize();
    if (readable > 0)
    {
        // 发送出去buffer->data + buffer->readPos 缓存区的位置+已经读到的位置
        //  管道破裂 -- 连接已经断开，服务器继续发数据，出现管道破裂 -- TCP协议
        //  当内核产生信号时，MSG_NOSIGNAL忽略，继续保持连接
        //  Linux的信号级别高，Linux大多数信号都会终止信号
        int count = send(socket, m_data + m_readPos, readable, MSG_NOSIGNAL);
        if (count > 0)
        {
            // 往后移动未读缓存区位置
            m_readPos += count;

            // 稍微休眠1微秒
            usleep(1);
        }
        return count;
    }
}

int Buffer::sendData(int cfd, int fd, off_t offset, int size)
{
    int count = 0;
    while(offset < size){
        // 系统函数，发送文件，linux内核提供的sendfile 也能减少拷贝次数
		//  sendfile发送文件效率高，而文件目录使用send
		// 通信文件描述符，打开文件描述符，fd对应的文件偏移量一般为空，
		// 单独单文件出现发送不全，offset会自动修改当前读取位置
        int ret = (int)sendfile(cfd, fd, &offset, (size_t)(size - offset));
        if(ret == -1 && errno == EAGAIN){
            printf("not data ....");
            perror("sendfile");
        }
        count += (int)offset;
    }
    return count;
}
