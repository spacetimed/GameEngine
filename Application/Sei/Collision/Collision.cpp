#include "Collision.h"
#include <cfloat>
#include <algorithm>

namespace Sei::Player::Collision
{
    bool overlap(const AABB& a, const AABB& b)
    {
        return a.min.x < b.max.x && a.max.x > b.min.x &&
               a.min.y < b.max.y && a.max.y > b.min.y &&
               a.min.z < b.max.z && a.max.z > b.min.z;
    }

    Hit sweep(const AABB& movingBox, const DirectX::XMFLOAT3& displacement, const AABB& obstacle)
    {
        if (overlap(movingBox, obstacle)) return { true, 0.0f, {} };
        const float min[] = { movingBox.min.x, movingBox.min.y, movingBox.min.z };
        const float max[] = { movingBox.max.x, movingBox.max.y, movingBox.max.z };
        const float otherMin[] = { obstacle.min.x, obstacle.min.y, obstacle.min.z };
        const float otherMax[] = { obstacle.max.x, obstacle.max.y, obstacle.max.z };
        const float delta[] = { displacement.x, displacement.y, displacement.z };
        float entry = -FLT_MAX, exit = FLT_MAX;
        int hitAxis = -1;
        for (int axis = 0; axis < 3; ++axis)
        {
            if (delta[axis] == 0.0f)
            {
                // Touching alone doesn't block movement parallel to a surface.
                if (max[axis] <= otherMin[axis] || min[axis] >= otherMax[axis]) return {};
                continue;
            }
            float near = (otherMin[axis] - max[axis]) / delta[axis];
            float far = (otherMax[axis] - min[axis]) / delta[axis];
            if (delta[axis] < 0.0f) std::swap(near, far);
            if (near > entry) { entry = near; hitAxis = axis; }
            exit = (std::min)(exit, far);
        }
        if (hitAxis < 0 || entry < 0.0f || entry > 1.0f || entry >= exit || exit <= 0.0f) return {};
        DirectX::XMFLOAT3 normal = {};
        const float direction = delta[hitAxis] > 0.0f ? -1.0f : 1.0f;
        if (hitAxis == 0) normal.x = direction;
        if (hitAxis == 1) normal.y = direction;
        if (hitAxis == 2) normal.z = direction;
        return { true, entry, normal };
    }

    AABB transform(const AABB& box, const DirectX::XMFLOAT4X4& matrix)
    {
        using namespace DirectX;
        auto min = XMVectorReplicate(FLT_MAX);
        auto max = XMVectorReplicate(-FLT_MAX);
        for (int corner = 0; corner < 8; ++corner)
        {
            const auto point = XMVector3TransformCoord(XMVectorSet(
                corner & 1 ? box.max.x : box.min.x,
                corner & 2 ? box.max.y : box.min.y,
                corner & 4 ? box.max.z : box.min.z, 1), XMLoadFloat4x4(&matrix));
            min = XMVectorMin(min, point);
            max = XMVectorMax(max, point);
        }
        AABB result;
        XMStoreFloat3(&result.min, min);
        XMStoreFloat3(&result.max, max);
        return result;
    }
}
