/*
 * @Description: 唤醒事件类实现
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-05 20:51:19
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-05 20:55:59
 */
#include <unistd.h>
#include "netrpc/net/wakeup_fd_event.h"
#include "netrpc/common/log.h"

namespace netrpc
{

    // 构造函数，初始化基类 FdEvent 并传递文件描述符 fd
    WakeUpFdEvent::WakeUpFdEvent(int fd) : FdEvent(fd)
    {
    }

    // 析构函数
    WakeUpFdEvent::~WakeUpFdEvent()
    {
    }

    // 唤醒函数
    void WakeUpFdEvent::wakeup()
    {
        char buf[8];                  // 定义一个长度为 8 字节的缓冲区
        int rt = write(m_fd, buf, 8); // 向文件描述符 m_fd 写入 8 字节数据

        // 检查写入的字节数是否为 8，如果少于 8，记录错误日志
        if (rt != 8)
        {
            ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", m_fd);
        }
        // 如果成功写入 8 字节，记录调试日志
        DEBUGLOG("successfully wrote 8 bytes");
    }
};