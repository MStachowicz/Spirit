#include "AABB.hpp"

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

#include <algorithm>

namespace Geometry
{
    AABB::AABB()
        : mMin{0.f, 0.f, 0.f}
        , mMax{0.f, 0.f, 0.f}
    {}
    AABB::AABB(const float& pLowX, const float& pHighX, const float& pLowY, const float& pHighY, const float& pLowZ, const float& pHighZ)
        : mMin{pLowX, pLowY, pLowZ}
        , mMax{pHighX, pHighY, pHighZ}
    {}
    AABB::AABB(const glm::vec3& pLowPoint, const glm::vec3& pHighPoint)
        : mMin{pLowPoint}
        , mMax{pHighPoint}
    {}
    glm::vec3 AABB::getSize() const
    {
        return mMax - mMin;
    }
    glm::vec3 AABB::getCenter() const
    {
        return (mMin + mMax) / 2.f;
    }

    bool AABB::contains(const AABB& pAABB) const
    {
        return
            mMin.x <= pAABB.mMax.x &&
            mMax.x >= pAABB.mMin.x &&
            mMin.y <= pAABB.mMax.y &&
            mMax.y >= pAABB.mMin.y &&
            mMin.z <= pAABB.mMax.z &&
            mMax.z >= pAABB.mMin.z;
    }
    bool AABB::operator== (const AABB& pOther) const
    {
        return mMin == pOther.mMin && mMax == pOther.mMax;
    };

    AABB AABB::unite(const AABB& pAABB, const AABB& pAABB2)
    {
        return {
            std::min(pAABB.mMin.x, pAABB2.mMin.x),
            std::max(pAABB.mMax.x, pAABB2.mMax.x),
            std::min(pAABB.mMin.y, pAABB2.mMin.y),
            std::max(pAABB.mMax.y, pAABB2.mMax.y),
            std::min(pAABB.mMin.z, pAABB2.mMin.z),
            std::max(pAABB.mMax.z, pAABB2.mMax.z)};
    }
    AABB AABB::unite(const AABB& pAABB, const glm::vec3& pPoint)
    {
        return {
            std::min(pAABB.mMin.x, pPoint.x),
            std::max(pAABB.mMax.x, pPoint.x),
            std::min(pAABB.mMin.y, pPoint.y),
            std::max(pAABB.mMax.y, pPoint.y),
            std::min(pAABB.mMin.z, pPoint.z),
            std::max(pAABB.mMax.z, pPoint.z)};
    }
    AABB AABB::transform(const AABB& pAABB, const glm::vec3& pTranslate, const glm::mat4& pTransform)
    {
        // Reference: Real-Time Collision Detection (Christer Ericson)
        // Each vertex of transformedAABB is a combination of three transformed min and max values from pAABB.
        // The minimum extent is the sum of all the smallers terms, the maximum extent is the sum of all the larger terms.
        // Translation doesn't affect the size calculation of the new AABB so can be added in.
        AABB transformedAABB;

        // For all 3 axes
        for (int i = 0; i < 3; i++)
        {
            // Apply translation
            transformedAABB.mMin[i] = transformedAABB.mMax[i] = pTranslate[i];

            // Form extent by summing smaller and larger terms respectively.
            for (int j = 0; j < 3; j++)
            {
                const float e = pTransform[j][i] * pAABB.mMin[j];
                const float f = pTransform[j][i] * pAABB.mMax[j];

                if (e < f)
                {
                    transformedAABB.mMin[i] += e;
                    transformedAABB.mMax[i] += f;
                }
                else
                {
                    transformedAABB.mMin[i] += f;
                    transformedAABB.mMax[i] += e;
                }
            }
        }

        return transformedAABB;
    }
} // namespace Geometry