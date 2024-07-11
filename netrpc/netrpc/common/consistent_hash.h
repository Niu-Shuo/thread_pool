/*
 * @Description: 一致性哈希类声明
 * @Version: 
 * @Author: Niu Shuo
 * @Date: 2024-06-03 20:26:11
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-03 21:00:44
 */
#ifndef NETRPC_CONSISTENT_HASH_H
#define NETRPC_CONSISTENT_HASH_H

#include <map>

namespace netrpc
{
    // 一致性哈希类，用于实现一致性哈希算法
    class ConsistentHash
    {
    public:
        // 构造函数，传入真实节点个数和每个节点关联的虚拟节点个数
        ConsistentHash(int node_num, int virtual_node_num);

        // 析构函数
        ~ConsistentHash();

        // 初始化一致性哈希环，包括虚拟节点的创建
        void Initialize();

        // 根据键值获取对应的服务器索引
        size_t GetServerIndex(const char *key);

        // 删除指定索引的节点
        void DeleteNode(const int index);

        // 添加新节点
        void AddNewNode(const int index);

    private:
        // 存储虚拟节点和其对应的真实节点索引
        std::map<uint32_t, size_t> server_nodes_;

        // 真实节点个数
        int node_num_;

        // 每个真实节点关联的虚拟节点个数
        int virtual_node_num_;
    };
};

#endif