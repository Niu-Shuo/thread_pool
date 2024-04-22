#ifndef REACTOR_THREAD_POOL_HTTPRESPONSE_H
#define REACTOR_THREAD_POOL_HTTPRESPONSE_H

#include "Buffer.h"
#include <map>

// 状态码枚举
enum class StatusCode
{
    Unknow,
    OK = 200,
    MovedPermannently = 301,
    MovedTemporarily = 302,
    BadRequest = 400,
    NotFund = 404
};

/*
    定义一个函数指针，用来组织要回复给客户端的数据块
    fileName要给客户端回复的静态资源内容
    sendBuf 内存，存储发送数据的Buffer缓冲
    socket 文件描述符，用于通信，将sendBuf数据发送给客户端
*/

// 定义http响应，添加响应头，准备响应数据
class HttpResponse
{
public:
    
private:
    StatusCode m_statusCode;                      // 状态行： 状态码，状态描述，http协议
    std::string m_fileName;                       // 文件名
    std::map<std::string, std::string> m_headers; // 响应头 - 键值对(存储响应头内部所有数据
    const std::map<int, std::string> m_info =     // 定义状态码和描述对应关系
        {
            {200, "OK"},
            {301, "MovedPermannently"},
            {302, "MovedTemporarily"},
            {400, "BadRequest"},
            {404, "NotFound"}};
};

#endif