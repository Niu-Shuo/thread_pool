/*
 * @Description: Config.h配置类
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 16:52:32
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-07-03 20:45:42
 */
#ifndef NETRPC_COMMON_CONFIG_H
#define NETRPC_COMMON_CONFIG_H

#include <map>
#include <tinyxml/tinyxml.h>
#include "netrpc/net/tcp/net_addr.h"

namespace netrpc
{
    // 定义RpcStub结构体，包含RPC服务的基本信息
    struct RpcStub
    {
        std::string name;         // RPC服务名称
        NetAddr::NetAddrPtr addr; // RPC服务地址，使用智能指针管理
        int timeout{20000};       // 超时时间，默认20000毫秒
    };

    // 配置类，使用单例模式
    class Config
    {
    public:
        // 初始化配置，解析XML文件
        void Init(const char *xmlfile);
        // 构造函数，接受XML文件路径
        // Config(const char *xmlfile);
        // 删除拷贝构造函数和赋值运算符，防止拷贝
        Config(const Config &) = delete;
        Config &operator=(const Config &) = delete;
        // 获取单例实例
        static Config &GetInst()
        {
            static Config inst;
            return inst;
        }
        // 析构函数，清理资源
        ~Config();

    public:
        // 配置项成员变量
        std::string m_log_level{"DEBUG"};           // 日志级别
        std::string m_log_file_name;                // 日志文件名
        std::string m_log_file_path;                // 日志文件路径
        int m_log_max_file_size{0};                 // 日志文件最大大小
        int m_log_sync_interval{0};                 // 日志同步间隔
        std::string m_ip;                           // 服务IP地址
        int m_port{0};                              // 服务端口
        int m_io_threads{0};                        // IO线程数
        TiXmlDocument *m_xml_document{nullptr};     // XML文档指针
        std::map<std::string, RpcStub> m_rpc_stubs; // 存储RPC服务的映射
        std::string m_zk_ip{"127.0.0.1"};           // ZooKeeper服务IP
        std::string m_zk_port{"0"};                 // ZooKeeper服务端口
    private:
        // 私有构造函数，防止直接实例化
        Config() = default;
    };
};

#endif