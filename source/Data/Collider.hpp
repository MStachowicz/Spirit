#pragma once

#include "glm/vec3.hpp"

namespace Data
{
    struct BoundingBox
    {
        BoundingBox() // const double& pBottom, const double& pTop, const double& pLeft, const double& pRight
        : mBottom(1.0)
        , mTop(1.0)
        , mLeft(1.0)
        , mRight(1.0)
        {}

        double mBottom;
        double mTop;
        double mLeft;
        double mRight;

        bool operator== (const BoundingBox& pOther) const
        {
            return mBottom == pOther.mBottom &&
                mTop == pOther.mTop &&
                mLeft == pOther.mLeft &&
                mRight == pOther.mRight;
        };
        bool operator != (const BoundingBox& pOther) const { return !(*this == pOther); }
    };

    struct Collider
    {
        BoundingBox mBoundingBox;

        bool operator== (const Collider& pOther)  const { return mBoundingBox == pOther.mBoundingBox; };
        bool operator != (const Collider& pOther) const { return !(*this == pOther); }
        void DrawImGui();
    };
}