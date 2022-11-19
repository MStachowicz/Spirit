#include "BoundingBox.hpp"

#include <algorithm>

namespace Collision
{
    BoundingBox unite(const BoundingBox& pBoundingBox, const BoundingBox& pBoundingBox2)
    {
        return {
            std::min(pBoundingBox.mLowX, pBoundingBox2.mLowX),
            std::max(pBoundingBox.mHighX, pBoundingBox2.mHighX),
            std::min(pBoundingBox.mLowY, pBoundingBox2.mLowY),
            std::max(pBoundingBox.mHighY, pBoundingBox2.mHighY),
            std::min(pBoundingBox.mLowZ, pBoundingBox2.mLowZ),
            std::max(pBoundingBox.mHighZ, pBoundingBox2.mHighZ)};
    }
} // namespace Collision