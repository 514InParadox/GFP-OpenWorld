#ifndef _UTILS_BVH_H
#define _UTILS_BVH_H

#include <vector>
#include <memory>
#include "utils/bounding_box.hpp"

struct BVHNode {
    BoundingBox           box;
    int                   left   = -1;   ///< 左孩子下标；-1 表示叶子
    int                   right  = -1;   ///< 右孩子
    int                   object = -1;   ///< 叶子：对应 CollisionSystem::_objects 的索引
};

/// @brief 线性化 BVH，适合 CPU 遍历
class BVHTree {
public:
    /// 传入所有物体的 world-space AABB，构建 BVH
    explicit BVHTree(const std::vector<BoundingBox>& boxes);

    /// 返回所有可能相交的索引对 (i, j)（i < j）
    std::vector<std::pair<int,int>> queryPairs() const;

private:
    int  buildRecursive(std::vector<int>& indices, int depth);
    void collectPairs(int nodeIdxA, int nodeIdxB,
                      std::vector<std::pair<int,int>>& out) const;

    std::vector<BVHNode>         _nodes;
    const std::vector<BoundingBox>* _boxArray {nullptr};
};

#endif
