/*
 * @Description: 一致性哈希类实现
 * @Version:
 * @Author: Niu Shuo
 * @Date: 2024-06-03 20:26:19
 * @LastEditors: Niu Shuo
 * @LastEditTime: 2024-06-03 21:00:18
 */
#include <string.h>
#include <sstream>
#include "netrpc/common/consistent_hash.h"
#include "netrpc/common/enc/murmurhash.h"

namespace netrpc
{
    // ConsistentHash 类的构造函数
    ConsistentHash::ConsistentHash(int node_num, int virtual_node_num)
    {
        node_num_ = node_num;                 // 初始化真实节点的数量
        virtual_node_num_ = virtual_node_num; // 初始化每个真实节点关联的虚拟节点数量
    }

    // ConsistentHash 类的析构函数
    ConsistentHash::~ConsistentHash()
    {
        server_nodes_.clear(); // 清空 server_nodes_ 映射表
    }

    // 初始化一致性哈希，为每个真实节点创建虚拟节点
    void ConsistentHash::Initialize()
    {
        for (int ii = 0; ii < node_num_; ++ii)
        {
            for (int jj = 0; jj < virtual_node_num_; ++jj)
            {
                std::stringstream node_key;
                node_key << "SHARD-" << ii << "-NODE-" << jj;                               // 为每个虚拟节点生成唯一的键
                auto partition = murmur3_32(node_key.str().c_str(), node_key.str().size()); // 计算键的哈希值
                server_nodes_.insert(std::pair<uint32_t, size_t>(partition, ii));           // 将虚拟节点插入 server_nodes_ 映射表
            }
        }
    }

    // 使用一致性哈希获取给定键对应的服务器索引
    size_t ConsistentHash::GetServerIndex(const char *key)
    {
        auto partition = murmur3_32(key, strlen(key));  // 计算键的哈希值
        auto it = server_nodes_.lower_bound(partition); // 查找哈希值大于或等于给定哈希值的服务器节点
        if (it == server_nodes_.end())
        {
            return server_nodes_.begin()->second; // 如果找不到服务器节点，则返回第一个服务器节点
        }
        return it->second; // 返回服务器节点的索引
    }

    // 从一致性哈希中删除节点
    void ConsistentHash::DeleteNode(const int index)
    {
        for (int jj = 0; jj < virtual_node_num_; ++jj)
        {
            std::stringstream node_key;
            node_key << "SHARD-" << index << "-NODE-" << jj;                                     // 生成要删除的虚拟节点的键
            auto partition = murmur3_32(node_key.str().c_str(), strlen(node_key.str().c_str())); // 计算键的哈希值
            auto it = server_nodes_.find(partition);                                             // 在 server_nodes_ 映射表中查找虚拟节点
            if (it != server_nodes_.end())
            {
                server_nodes_.erase(it); // 从 server_nodes_ 映射表中删除虚拟节点
            }
        }
    }

    // 向一致性哈希中添加新节点
    void ConsistentHash::AddNewNode(const int index)
    {
        for (int jj = 0; jj < virtual_node_num_; ++jj)
        {
            std::stringstream node_key;
            node_key << "SHARD-" << index << "-NODE-" << jj;                                     // 生成新虚拟节点的键
            auto partition = murmur3_32(node_key.str().c_str(), strlen(node_key.str().c_str())); // 计算键的哈希值
            server_nodes_.insert(std::pair<uint32_t, size_t>(partition, index));                 // 将新虚拟节点插入 server_nodes_ 映射表
        }
    }
};
