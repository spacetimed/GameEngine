#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace Sei
{
    struct Mesh
    {
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
        UINT indexCount = 0;
    };
}