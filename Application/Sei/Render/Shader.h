#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace Sei
{
    struct Shader
    {
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    };
}
