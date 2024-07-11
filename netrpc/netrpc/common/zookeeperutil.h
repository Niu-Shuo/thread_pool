/*
 * @Description: zookeeper工具类
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-13 18:48:13
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-14 21:34:22
 */

#ifndef NETRPC_COMMON_ZOOKEEPERUTIL_H
#define NETRPC_COMMON_ZOOKEEPERUTIL_H

#include <zookeeper/zookeeper.h>
#include <string>

namespace netrpc {

    // 封装的zk客户端类
    class ZkClient {
    public:
        ZkClient(); // 构造函数
        ~ZkClient(); // 析构函数

        // 启动连接到zkserver
        void Start();

        // 在zkserver上根据指定的path创建znode节点
        void Create(const char *path, const char *data, int datalen, int state = 0);

        // 获取指定路径下znode节点的数据
        std::string GetData(const char *path);

    private:
        zhandle_t *m_zhandle; // Zookeeper客户端句柄
    };

}; // namespace netrpc

#endif // NETRPC_COMMON_ZOOKEEPERUTIL_H