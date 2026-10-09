#include "HUD.h"

#include <d2d1_1.h>
#include <dwrite.h>
#include <wrl/client.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace
{
    using Microsoft::WRL::ComPtr;
    ComPtr<ID2D1DeviceContext> target;
    ComPtr<IDWriteFactory> writeFactory;
    ComPtr<IDWriteTextFormat> format;
    ComPtr<ID2D1SolidColorBrush> brush;
    float currentFontSize = 0;
    std::vector<const Sei::HUD::Section*> sections;

    std::wstring WideText(const std::string& text)
    {
        const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(),
            static_cast<int>(text.size()), nullptr, 0);
        std::wstring wide(length, L'\0');
        if (length)
            MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wide.data(), length);
        return wide;
    }
}

namespace Sei::HUD
{
    bool Initialize(IDXGISurface* surface)
    {
        ComPtr<ID2D1Factory1> factory;
        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory.GetAddressOf())))
            return false;
        ComPtr<IDXGIDevice> dxgiDevice;
        if (FAILED(surface->GetDevice(IID_PPV_ARGS(dxgiDevice.GetAddressOf())))) return false;
        ComPtr<ID2D1Device> device;
        if (FAILED(factory->CreateDevice(dxgiDevice.Get(), device.GetAddressOf()))) return false;
        if (FAILED(device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, target.GetAddressOf())))
            return false;

        const auto properties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), 96.0f, 96.0f);
        ComPtr<ID2D1Bitmap1> bitmap;
        if (FAILED(target->CreateBitmapFromDxgiSurface(surface, &properties, bitmap.GetAddressOf())))
            return false;
        target->SetTarget(bitmap.Get());
        target->SetDpi(96.0f, 96.0f); // Section positions and font sizes use pixels.
        if (FAILED(target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), brush.GetAddressOf())))
            return false;
        return SUCCEEDED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(writeFactory.GetAddressOf())));
    }

    void BindSection(const Section& section)
    {
        for (const auto* existing : sections)
            if (existing == &section) return;
        sections.push_back(&section);
    }

    bool Draw()
    {
        if (sections.empty()) return true;
        if (!target || !writeFactory) return false;
        target->BeginDraw();
        const auto size = target->GetSize();
        for (const auto* section : sections)
        {
            if (!section->visible) continue;
            if (section->fontSize <= 0) continue;
            if (!format || currentFontSize != section->fontSize)
            {
                format.Reset();
                if (FAILED(writeFactory->CreateTextFormat(L"Consolas", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
                    DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, section->fontSize,
                    L"en-us", format.GetAddressOf())))
                {
                    target->EndDraw();
                    return false;
                }
                format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
                currentFontSize = section->fontSize;
            }
            const auto& color = section->textColor;
            brush->SetColor(D2D1::ColorF(color.x, color.y, color.z, color.w));
            std::string line;
            bool firstItem = true;
            for (const auto& item : section->items)
            {
                if (!item.value) continue;
                if (!firstItem) line += " ; ";
                line += item.key.empty() ? *item.value : item.key + ": " + *item.value;
                firstItem = false;
            }
            const auto text = WideText(line);
            const auto bounds = D2D1::RectF(section->textPosition.x, section->textPosition.y,
                size.width, size.height);
            target->DrawText(text.c_str(), static_cast<UINT32>(text.size()), format.Get(),
                bounds, brush.Get());
        }
        return SUCCEEDED(target->EndDraw());
    }

    void Shutdown()
    {
        sections.clear();
        brush.Reset();
        format.Reset();
        writeFactory.Reset();
        target.Reset();
        currentFontSize = 0;
    }
}
