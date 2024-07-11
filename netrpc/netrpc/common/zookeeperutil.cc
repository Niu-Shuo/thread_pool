/*
 * @Description: 实现了一个封装了 Zookeeper 客户端操作的类 ZkClient，包括连接、创建节点、获取节点数据等功能。
 *               使用全局的 watcher 函数处理会话事件通知。
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-13 18:51:05
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-13 19:38:30
 */

#include "netrpc/common/zookeeperutil.h"
#include "netrpc/common/config.h"
#include <semaphore.h>
#include <iostream>

namespace netrpc {

    // 全局的watcher观察器，处理zkserver发送给zkclient的通知
    void global_watcher(zhandle_t *zh, int type, int state, const char* path, void* watcherCtx) {
        // 当事件类型为会话事件时处理
        if (type == ZOO_SESSION_EVENT) {
            sem_t *sem = (sem_t*)zoo_get_context(zh); // 获取信号量
            sem_post(sem); // 发送信号，唤醒等待的线程
        }
    }

    ZkClient::ZkClient() : m_zhandle(nullptr)
    {
    }

    ZkClient::~ZkClient()
    {
        if (m_zhandle != nullptr) {
            zookeeper_close(m_zhandle); // 关闭Zookeeper句柄，释放资源
        }
    }

    // 启动连接到zkserver
    void ZkClient::Start()
    {
        // 从配置中获取zkserver的IP和端口
        std::string host = Config::GetInst().m_zk_ip;
        std::string port = Config::GetInst().m_zk_port;
        std::string connstr = host + ":" + port;

        // 初始化Zookeeper客户端，连接zkserver
        m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
        if (m_zhandle == nullptr) {
            std::cout << "zookeeper_init error!" << std::endl;
            exit(EXIT_FAILURE); // 初始化失败，退出程序
        }

        sem_t sem;
        sem_init(&sem, 0, 0); // 初始化信号量
        zoo_set_context(m_zhandle, &sem); // 设置Zookeeper句柄的上下文为信号量
        sem_wait(&sem); // 等待信号量，阻塞当前线程，直到会话事件完成
        std::cout << "zookeeper_init success!" << std::endl;
    }

    // 在zkserver上根据指定的path创建znode节点
    void ZkClient::Create(const char *path, const char *data, int datalen, int state)
    {
        char path_buffer[128];
        int bufferlen = sizeof(path_buffer);
        int flag;

        // 判断path表示的znode节点是否存在
        flag = zoo_exists(m_zhandle, path, 0, nullptr);

        // 如果znode节点不存在，则创建
        if (flag == ZNONODE) {
            flag = zoo_create(m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
            if (flag == ZOK) {
                std::cout << "znode create success... path:" << path << std::endl;
            } else {
                std::cout << "flag:" << flag << std::endl;
                std::cout << "znode create error... path:" << path << std::endl;
                exit(EXIT_FAILURE); // 创建失败，退出程序
            }
        }
    }

    // 根据指定的path获取znode节点的值
    std::string ZkClient::GetData(const char *path)
    {
        char buffer[64];
        int bufferlen = sizeof(buffer);
        int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);

        // 获取失败时输出错误信息并返回空字符串
        if (flag != ZOK) {
            std::cout << "get znode error... path:" << path << std::endl;
            return "";
        } else {
            return buffer; // 返回获取到的节点值
        }
    }

}; // namespace netrpc