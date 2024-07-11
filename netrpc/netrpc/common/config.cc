/*
 * @Description: config.cc 配置类实现
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 20:11:07
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-14 18:27:53
 */
#include <tinyxml/tinyxml.h>
#include <typeinfo>
#include <iostream>
#include "netrpc/common/config.h"
#include "config.h"

// 宏定义，用于读取XML节点并检查是否读取成功
#define READ_XML_NODE(name, parent)                                             \
    TiXmlElement *name##_node = parent->FirstChildElement(#name);               \
    if (!name##_node)                                                           \
    {                                                                           \
        printf("Start netrpc server error, failed to read node [%s]\n", #name); \
        exit(0);                                                                \
    }

// 宏定义，用于从XML节点读取字符串值并检查是否读取成功
#define READ_STR_FROM_XML_NODE(name, parent)                                         \
    TiXmlElement *name##_node = parent->FirstChildElement(#name);                    \
    if (!name##_node || !name##_node->GetText())                                     \
    {                                                                                \
        printf("Start netrpc server error, failed to read config file %s\n", #name); \
        exit(0);                                                                     \
    }                                                                                \
    std::string name##_str = std::string(name##_node->GetText());

namespace netrpc
{
    Config::~Config()
    {
        // 析构函数，释放XML文档指针
        if (m_xml_document)
        {
            delete m_xml_document;
            m_xml_document = nullptr;
        }
    }
    
    void Config::Init(const char *xmlfile)
    {
        // 初始化配置，从XML文件读取配置
        m_xml_document = new TiXmlDocument();

        // 尝试加载XML文件
        bool rt = m_xml_document->LoadFile(xmlfile);
        if (!rt)
        {
            // 如果加载失败，打印错误信息并退出程序
            printf("Start netrpc server error, failed to read config file %s, error info[%s] \n", xmlfile, m_xml_document->ErrorDesc());
            exit(0);
        }

        // 读取根节点
        READ_XML_NODE(root, m_xml_document);
        // 读取日志配置节点
        READ_XML_NODE(log, root_node);
        // 读取服务器配置节点
        READ_XML_NODE(server, root_node);
        // 读取ZooKeeper配置节点
        READ_XML_NODE(zk_config, root_node);

        // 从日志配置节点读取字符串值
        READ_STR_FROM_XML_NODE(log_level, log_node);
        READ_STR_FROM_XML_NODE(log_file_name, log_node);
        READ_STR_FROM_XML_NODE(log_file_path, log_node);
        READ_STR_FROM_XML_NODE(log_max_file_size, log_node);
        READ_STR_FROM_XML_NODE(log_sync_interval, log_node);

        // 设置日志配置
        m_log_level = log_level_str;
        m_log_file_name = log_file_name_str;
        m_log_file_path = log_file_path_str;
        m_log_max_file_size = std::atoi(log_max_file_size_str.c_str());
        m_log_sync_interval = std::atoi(log_sync_interval_str.c_str());

        // 打印日志配置信息
        printf("LOG -- CONFIG LEVEL[%s], FILE_NAME[%s],FILE_PATH[%s] MAX_FILE_SIZE[%d B], SYNC_INTEVAL[%d ms]\n",
               m_log_level.c_str(), m_log_file_name.c_str(), m_log_file_path.c_str(), m_log_max_file_size, m_log_sync_interval);

        // 从服务器配置节点读取字符串值
        READ_STR_FROM_XML_NODE(ip, server_node);
        READ_STR_FROM_XML_NODE(port, server_node);
        READ_STR_FROM_XML_NODE(io_threads, server_node);

        // 设置服务器配置
        m_ip = ip_str;
        m_port = std::atoi(port_str.c_str());
        m_io_threads = std::atoi(io_threads_str.c_str());

        // 从ZooKeeper配置节点读取字符串值
        READ_STR_FROM_XML_NODE(zk_ip, zk_config_node);
        READ_STR_FROM_XML_NODE(zk_port, zk_config_node);
        m_zk_ip = zk_ip_str;
        m_zk_port = zk_port_str;

        // 读取RPC服务配置
        TiXmlElement *stubs_node = root_node->FirstChildElement("stubs");

        if (stubs_node)
        {
            // 遍历所有的rpc_server节点
            for (TiXmlElement *node = stubs_node->FirstChildElement("rpc_server"); node; node = node->NextSiblingElement("rpc_server"))
            {
                RpcStub stub;
                stub.name = std::string(node->FirstChildElement("name")->GetText());
                stub.timeout = std::atoi(node->FirstChildElement("timeout")->GetText());

                std::string ip = std::string(node->FirstChildElement("ip")->GetText());
                uint16_t port = std::atoi(node->FirstChildElement("port")->GetText());
                stub.addr = std::make_shared<IPNetAddr>(ip, port);

                // 将RPC服务配置信息存入map中
                m_rpc_stubs.insert(std::make_pair(stub.name, stub));
            }
        }

        // 打印服务器配置信息
        printf("Server -- PORT[%d], IO Threads[%d]\n", m_port, m_io_threads);
    }

}