/*
 * @Description: 异常
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-03 21:02:26
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-03 21:10:45
 */
#ifndef NETRPC_COMMON_EXCEPTION_H
#define NETRPC_COMMON_EXCEPTION_H

#include <exception> // 包含异常处理的头文件
#include <string>    // 包含字符串处理的头文件

namespace netrpc
{

    class NetrpcException : public std::exception // 继承自标准异常类
    {
    public:
        // 构造函数，接受错误码和错误信息作为参数
        NetrpcException(int error_code, const std::string &error_info) : m_error_code(error_code), m_error_info(error_info) {}

        // 异常处理函数
        // 当 EventLoop 捕获到 NetrpcException 及其子类对象的异常时，会执行该函数
        virtual void handle() = 0;

        virtual ~NetrpcException(){}; // 虚析构函数

        // 获取错误码的方法
        int errorCode()
        {
            return m_error_code;
        }

        // 获取错误信息的方法
        std::string errorInfo()
        {
            return m_error_info;
        }

    protected:
        int m_error_code{0};      // 错误码
        std::string m_error_info; // 错误信息
    };

}

#endif