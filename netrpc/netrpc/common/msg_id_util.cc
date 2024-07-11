/*
 * @Description: 生成消息 ID 的工具类实现
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 21:08:22
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-14 21:25:12
 */
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "netrpc/common/msg_id_util.h"
#include "netrpc/common/log.h"

namespace netrpc
{

    static int g_msg_id_length = 20; // 消息 ID 的长度
    static int g_random_fd = -1;     // 用于随机数生成的文件描述符

    static thread_local std::string t_msg_id_no;     // 线程局部变量，存储当前线程的消息 ID
    static thread_local std::string t_max_msg_id_no; // 线程局部变量，存储当前线程的最大消息 ID

    // 生成消息 ID
    std::string MsgIDUtil::GenMsgID()
    {
        // 如果当前消息 ID 为空或已达到最大值，则重新生成
        if (t_msg_id_no.empty() || t_msg_id_no == t_max_msg_id_no)
        {
            // 如果随机数文件描述符未打开，则打开 /dev/urandom 文件
            if (g_random_fd == -1)
            {
                g_random_fd = open("/dev/urandom", O_RDONLY);
            }
            std::string res(g_msg_id_length, 0); // 初始化长度为 g_msg_id_length 的字符串
            // 从随机数文件中读取 g_msg_id_length 个字符到 res 中
            if ((read(g_random_fd, &res[0], g_msg_id_length)) != g_msg_id_length)
            {
                ERRORLOG("read from /dev/urandom error");
                return "";
            }
            // 将每个字符转换为数字，并保证在 0-9 范围内
            for (int ii = 0; ii < g_msg_id_length; ++ii)
            {
                uint8_t x = ((uint8_t)(res[ii])) % 10;
                res[ii] = x + '0';
                t_max_msg_id_no += "9"; // 更新最大消息 ID
            }
            t_msg_id_no = res; // 更新当前线程的消息 ID
        }
        else // 如果当前消息 ID 还未达到最大值，则递增
        {
            size_t i = t_msg_id_no.length() - 1;
            while (t_msg_id_no[i] == '9' && i >= 0) // 从后往前找到第一个不是 '9' 的位置
            {
                i--;
            }
            if (i >= 0) // 如果找到了不是 '9' 的位置，则将该位置的字符递增
            {
                t_msg_id_no[i] += 1;
                for (size_t j = i + 1; j < t_msg_id_no.length(); ++j) // 将后面的字符全部设为 '0'
                {
                    t_msg_id_no[j] = '0';
                }
            }
        }

        return t_msg_id_no; // 返回生成的消息 ID
    }

}