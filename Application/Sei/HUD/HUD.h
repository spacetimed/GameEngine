#pragma once

#include "Section.h"
#include <dxgi.h>

namespace Sei::HUD
{
    bool Initialize(IDXGISurface* surface);
    void Shutdown();

    // The section and its value strings must outlive their registration.
    void BindSection(const Section& section);
    bool Draw();
}
