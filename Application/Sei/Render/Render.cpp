#include "Render.h"

#include <d3d11.h>
#include <wrl/client.h>

#include <iostream>

#pragma comment(lib, "d3d11.lib")

// keep GPU objects private
namespace
{
    using Microsoft::WRL::ComPtr;

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain> swapChain;
    ComPtr<ID3D11RenderTargetView> renderTarget;
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
        context->OMSetRenderTargets(1, &target, nullptr);

        // fill it with blue (sanity test)
        const float colour[] = { 0.1f, 0.2f, 0.4f, 1.0f };
        context->ClearRenderTargetView(target, colour);
    }

    void EndFrame()
    {
        // present completed frame
        swapChain->Present(1, 0);
    }

    void Shutdown()
    {
        // release/cleanup
        if (context) context->ClearState();
        renderTarget.Reset();
        swapChain.Reset();
        context.Reset();
        device.Reset();
    }
}
