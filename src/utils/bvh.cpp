#include "utils/bvh.hpp"
#include <algorithm>
#include <numeric>      // std::iota
#include <cfloat>       // FLT_MAX

/* ======= BoundingBox 辅助函数 ======= */
namespace {

inline BoundingBox makeEmptyBB() {
    return { glm::vec3( FLT_MAX), glm::vec3(-FLT_MAX) };
}

inline void encapsulate(BoundingBox& bb, const BoundingBox& rhs) {
    bb.min = glm::min(bb.min, rhs.min);
    bb.max = glm::max(bb.max, rhs.max);
}

inline glm::vec3 center(const BoundingBox& bb) {
    return 0.5f * (bb.min + bb.max);
}

inline bool intersects(const BoundingBox& a, const BoundingBox& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

} // namespace
/* ==================================== */

BVHTree::BVHTree(const std::vector<BoundingBox>& boxes) {
    _boxArray = &boxes;
    std::vector<int> idx(boxes.size());
    std::iota(idx.begin(), idx.end(), 0);
    buildRecursive(idx, 0);
}

/* ----------- 递归构建 ------------ */
int BVHTree::buildRecursive(std::vector<int>& indices, int /*depth*/) {

    int nodeIdx = static_cast<int>(_nodes.size());
    _nodes.push_back({});

    BoundingBox nodeBox = makeEmptyBB();
    for (int i : indices) encapsulate(nodeBox, (*_boxArray)[i]);
    _nodes[nodeIdx].box = nodeBox;

    /* 叶子（≤2 个对象） */
    if (indices.size() <= 2) {
        if (!indices.empty())
            _nodes[nodeIdx].object = indices[0];

        if (indices.size() == 2) {
            BVHNode child;
            child.box    = (*_boxArray)[indices[1]];
            child.object = indices[1];
            _nodes.push_back(child);
            _nodes[nodeIdx].left = static_cast<int>(_nodes.size()) - 1;
        }
        return nodeIdx;
    }

    /* 拆分最长轴 */
    glm::vec3 ext = nodeBox.max - nodeBox.min;
    int axis = ext.x > ext.y
                ? (ext.x > ext.z ? 0 : 2)
                : (ext.y > ext.z ? 1 : 2);

    std::sort(indices.begin(), indices.end(),
              [&](int a, int b) {
                  return center((*_boxArray)[a])[axis] <
                         center((*_boxArray)[b])[axis];
              });

    size_t mid = indices.size() / 2;
    std::vector<int> left (indices.begin(),           indices.begin()+mid);
    std::vector<int> right(indices.begin()+mid, indices.end());

    _nodes[nodeIdx].left  = buildRecursive(left , /*depth+1*/0);
    _nodes[nodeIdx].right = buildRecursive(right, /*depth+1*/0);
    return nodeIdx;
}

/* ----------- 生成候选对 ------------ */
std::vector<std::pair<int,int>> BVHTree::queryPairs() const {
    std::vector<std::pair<int,int>> pairs;
    if (_nodes.empty()) return pairs;
    collectPairs(0, 0, pairs);
    return pairs;
}

/* 深度优先遍历节点对，收集相交叶子对 */
void BVHTree::collectPairs(int a, int b,
                           std::vector<std::pair<int,int>>& out) const {
    if (a == -1 || b == -1) return;
    if (a > b) std::swap(a, b);                  // 保证 (a,b) 有序，防止重复
    if (!intersects(_nodes[a].box, _nodes[b].box)) return;

    bool aLeaf = (_nodes[a].object >= 0);
    bool bLeaf = (_nodes[b].object >= 0);

    if (aLeaf && bLeaf) {                        // 两叶 → 输出对象索引对
        int idxA = _nodes[a].object;
        int idxB = _nodes[b].object;
        if (idxA < idxB)
            out.emplace_back(idxA, idxB);
        return;
    }

    if (aLeaf) {                                 // 递归 a × (b 的孩子)
        collectPairs(a, _nodes[b].left , out);
        collectPairs(a, _nodes[b].right, out);
    } else if (bLeaf) {                          // (a 的孩子) × b
        collectPairs(_nodes[a].left , b, out);
        collectPairs(_nodes[a].right, b, out);
    } else {                                     // 都非叶，四组合
        collectPairs(_nodes[a].left , _nodes[b].left , out);
        collectPairs(_nodes[a].left , _nodes[b].right, out);
        collectPairs(_nodes[a].right, _nodes[b].left , out);
        collectPairs(_nodes[a].right, _nodes[b].right, out);
    }
}
