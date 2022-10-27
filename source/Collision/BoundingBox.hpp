#pragma once

namespace Collision
{
    // Return a bounding box encompassing both bounding boxes.
    BoundingBox unite(const BoundingBox& pBoundingBox, const BoundingBox& pBoundingBox2);

    struct BoundingBox
    {
        double mLowX;
        double mHighX;

        double mLowY;
        double mHighY;

        double mLowZ;
        double mHighZ;

        BoundingBox() : mLowX{0.0}, mHighX{0.0}, mLowY{0.0}, mHighY{0.0}, mLowZ{0.0}, mHighZ{0.0} {}
        BoundingBox(const double& pLowX, const double& pHighX, const double& pLowY, const double& pHighY, const double& pLowZ, const double& pHighZ)
            : mLowX(pLowX)
            , mHighX(pHighX)
            , mLowY(pLowY)
            , mHighY(pHighY)
            , mLowZ(pLowZ)
            , mHighZ(pHighZ)
        {}

        bool contains(const BoundingBox& pBoundingBox) const
        {
            return
                mLowX  <= pBoundingBox.mHighX &&
                mHighX >= pBoundingBox.mLowX  &&
                mLowY  <= pBoundingBox.mHighY &&
                mHighY >= pBoundingBox.mLowY  &&
                mLowZ  <= pBoundingBox.mHighZ &&
                mHighZ >= pBoundingBox.mLowZ;
        }

        bool operator== (const BoundingBox& pOther) const
        {
            return
                mLowX  == pOther.mLowX  &&
                mHighX == pOther.mHighX &&
                mLowY  == pOther.mLowY  &&
                mHighY == pOther.mHighY &&
                mLowZ  == pOther.mLowZ  &&
                mHighZ == pOther.mHighZ;
        };
        bool operator != (const BoundingBox& pOther) const { return !(*this == pOther); }
    };
}// namespace Collision