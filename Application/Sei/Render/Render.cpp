#include "Render.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include <iostream>
#include <utility> // for std::move

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// keep GPU objects private
namespace
{
    using Microsoft::WRL::ComPtr;

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11RenderTargetView> renderTarget;
    ComPtr<ID3D11DepthStencilView> depthTarget;
    ComPtr<ID3D11Buffer> transformBuffer;
    ComPtr<ID3D11RasterizerState> rasterizer;
    D3D11_VIEWPORT viewport = {};
}

namespace Sei::Render
{
    bool Initialize(HWND window)
    {
        // describe swap chain initialization
        DXGI_SWAP_CHAIN_DESC desc = {};
        desc.BufferCount = 1;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow = window;
        desc.SampleDesc.Count = 1;
        desc.Windowed = TRUE;
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        // create swap chain
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &desc,
            swapChain.GetAddressOf(),
            device.GetAddressOf(),
            nullptr,
            context.GetAddressOf()
        );
        if (FAILED(hr)) return false;

        // get back buffer texture
        ComPtr<ID3D11Texture2D> backBuffer;
        hr = swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
        if (FAILED(hr)) return false;

        // create render target view for accessing resource data
        hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTarget.GetAddressOf());
        if (FAILED(hr)) return false;

        // Match the depth buffer and viewport to the actual back buffer.
        D3D11_TEXTURE2D_DESC backDesc = {};
        backBuffer->GetDesc(&backDesc);

        D3D11_TEXTURE2D_DESC depthDesc = {};
        depthDesc.Width = backDesc.Width;
        depthDesc.Height = backDesc.Height;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc = backDesc.SampleDesc;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        ComPtr<ID3D11Texture2D> depthTexture;
        hr = device->CreateTexture2D(&depthDesc, nullptr, depthTexture.GetAddressOf());
        if (FAILED(hr)) return false;
        hr = device->CreateDepthStencilView(depthTexture.Get(), nullptr, depthTarget.GetAddressOf());
        if (FAILED(hr)) return false;

        viewport.Width = static_cast<float>(backDesc.Width);
        viewport.Height = static_cast<float>(backDesc.Height);
        viewport.MaxDepth = 1.0f;

        D3D11_BUFFER_DESC transformDesc = {};
        transformDesc.ByteWidth = sizeof(DirectX::XMFLOAT4X4);
        transformDesc.Usage = D3D11_USAGE_DEFAULT;
        transformDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        hr = device->CreateBuffer(&transformDesc, nullptr, transformBuffer.GetAddressOf());
        if (FAILED(hr)) return false;

        // Show both sides for now, regardless of the imported model's winding.
        D3D11_RASTERIZER_DESC rasterDesc = {};
        //rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
        rasterDesc.CullMode = D3D11_CULL_NONE;
        rasterDesc.DepthClipEnable = TRUE;
        hr = device->CreateRasterizerState(&rasterDesc, rasterizer.GetAddressOf());
        if (FAILED(hr)) return false;

        LogDebug();

        return true;
    }

    void LogDebug()
    {
        // print GPU to CLI just for sanity
        ComPtr<IDXGIDevice> dxgiDevice;
        ComPtr<IDXGIAdapter> adapter;
        DXGI_ADAPTER_DESC adapterDesc = {};

        if (SUCCEEDED(device.As(&dxgiDevice)) &&
            SUCCEEDED(dxgiDevice->GetAdapter(adapter.GetAddressOf())) &&
            SUCCEEDED(adapter->GetDesc(&adapterDesc)))
        {
            std::wcout << L"using GPU: " << adapterDesc.Description << L'\n';
        }

        std::cout << "renderer successfully initialized!\n";
    }

    void BeginFrame()
    {
        // select back buffer as drawing destination
        ID3D11RenderTargetView* target = renderTarget.Get();
        context->OMSetRenderTargets(1, &target, depthTarget.Get());
        context->OMSetDepthStencilState(nullptr, 0); // Default: depth test and writes enabled.
        context->RSSetViewports(1, &viewport);
        context->RSSetState(rasterizer.Get());

        // fill it with blue (sanity test)
        const float colour[] = { 0.1f, 0.2f, 0.4f, 1.0f };
        context->ClearRenderTargetView(target, colour);
        context->ClearDepthStencilView(depthTarget.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    void Draw(const Mesh& mesh, const Shader& shader, const DirectX::XMMATRIX& worldViewProjection)
    {
        // HLSL matrices use column-major storage by default.
        DirectX::XMFLOAT4X4 transform;
        DirectX::XMStoreFloat4x4(&transform, DirectX::XMMatrixTranspose(worldViewProjection));
        context->UpdateSubresource(transformBuffer.Get(), 0, nullptr, &transform, 0, 0);

        ID3D11Buffer* constants = transformBuffer.Get();
        context->VSSetConstantBuffers(0, 1, &constants);
        context->VSSetShader(shader.vertexShader.Get(), nullptr, 0);
        context->PSSetShader(shader.pixelShader.Get(), nullptr, 0);
        context->IASetInputLayout(shader.inputLayout.Get());

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        ID3D11Buffer* vertices = mesh.vertexBuffer.Get();
        context->IASetVertexBuffers(0, 1, &vertices, &stride, &offset);
        context->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(mesh.indexCount, 0, 0);
    }

    void EndFrame()
    {
        // present completed frame
        swapChain->Present(1, 0);
    }

    void Draw(const Mesh& mesh, const Shader& shader, const Camera& camera)
    {
        // This overload draws a mesh at the world origin.
        Draw(mesh, shader, camera.GetViewMatrix() * camera.GetProjectionMatrix());
    }

    void Shutdown()
    {
        // release/cleanup
        if (context) context->ClearState();
        rasterizer.Reset();
        transformBuffer.Reset();
        depthTarget.Reset();
        renderTarget.Reset();
        swapChain.Reset();
        context.Reset();
        device.Reset();
    }

    bool CreateShader(const std::filesystem::path& path, Shader& output, std::string& error)
    {
        error.clear();
        if (!device)
        {
            error = "Initialize the renderer before creating shaders.";
            return false;
        }

        // Compile the two entry points from the same HLSL file.
        auto compile = [&](const char* entry, const char* profile, ComPtr<ID3DBlob>& bytecode)
        {
            ComPtr<ID3DBlob> diagnostics;
            HRESULT hr = D3DCompileFromFile(
                path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                entry, profile, D3DCOMPILE_ENABLE_STRICTNESS, 0,
                bytecode.GetAddressOf(), diagnostics.GetAddressOf());

            if (FAILED(hr))
            {
                error = std::string("Could not compile ") + entry + " in " + path.string();
                if (diagnostics)
                {
                    error += "\n";
                    error.append(static_cast<const char*>(diagnostics->GetBufferPointer()),
                                 diagnostics->GetBufferSize());
                }
                else
                    error += " (HRESULT " + std::to_string(hr) + ")";
                return false;
            }
            return true;
        };

        ComPtr<ID3DBlob> vertexCode;
        ComPtr<ID3DBlob> pixelCode;
        if (!compile("VSMain", "vs_5_0", vertexCode) ||
            !compile("PSMain", "ps_5_0", pixelCode))
            return false;

        // Only replace the caller's shader after every step succeeds.
        Shader shader;
        HRESULT hr = device->CreateVertexShader(
            vertexCode->GetBufferPointer(), vertexCode->GetBufferSize(),
            nullptr, shader.vertexShader.GetAddressOf());
        if (FAILED(hr))
        {
            error = "Could not create vertex shader (HRESULT " + std::to_string(hr) + ")";
            return false;
        }

        hr = device->CreatePixelShader(
            pixelCode->GetBufferPointer(), pixelCode->GetBufferSize(),
            nullptr, shader.pixelShader.GetAddressOf());
        if (FAILED(hr))
        {
            error = "Could not create pixel shader (HRESULT " + std::to_string(hr) + ")";
            return false;
        }

        // Matches our current Vertex: three floats containing position.
        const D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
              D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        hr = device->CreateInputLayout(
            layout, 1, vertexCode->GetBufferPointer(), vertexCode->GetBufferSize(),
            shader.inputLayout.GetAddressOf());
        if (FAILED(hr))
        {
            error = "Could not create input layout (HRESULT " + std::to_string(hr) + ")";
            return false;
        }

        output = std::move(shader);
        return true;
    }

    bool CreateMesh(const MeshData& data, Mesh& output)
    {
        if (!device || data.vertices.empty() || data.indices.empty())
            return false;

        Mesh mesh;

        // Upload vertices.
        D3D11_BUFFER_DESC vertexDesc = {};
        vertexDesc.ByteWidth =
            static_cast<UINT>(data.vertices.size() * sizeof(Vertex));
        vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexData = {};
        vertexData.pSysMem = data.vertices.data();

        HRESULT hr = device->CreateBuffer(
            &vertexDesc,
            &vertexData,
            mesh.vertexBuffer.GetAddressOf()
        );

        if (FAILED(hr))
            return false;

        // Upload indices.
        D3D11_BUFFER_DESC indexDesc = {};
        indexDesc.ByteWidth =
            static_cast<UINT>(data.indices.size() * sizeof(std::uint32_t));
        indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexData = {};
        indexData.pSysMem = data.indices.data();

        hr = device->CreateBuffer(
            &indexDesc,
            &indexData,
            mesh.indexBuffer.GetAddressOf()
        );

        if (FAILED(hr))
            return false;

        mesh.indexCount = static_cast<UINT>(data.indices.size());

        output = std::move(mesh);
        return true;
    }
}
