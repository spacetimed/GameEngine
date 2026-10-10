#pragma once

/*
    ├── Sweep(playerBox, movement)
    ├── Overlap(playerBox)
    └── Raycast(origin, direction)
*/

#include <DirectXMath.h>

namespace Sei::Player::Collision
{
    struct AABB
    {
        DirectX::XMFLOAT3 min;
        DirectX::XMFLOAT3 max;
    };

    struct Hit
    {
        bool hit = false;
        float fraction = 1.0f;
        DirectX::XMFLOAT3 normal = {};
    };

    bool overlap(const AABB& a, const AABB& b);
    AABB transform(const AABB& box, const DirectX::XMFLOAT4X4& matrix);

    Hit sweep(
        const AABB& movingBox,
        const DirectX::XMFLOAT3& displacement,
        const AABB& obstacle
    );
}
