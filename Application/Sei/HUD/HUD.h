#pragma once

#include "Section.h"
#include <dxgi.h>

namespace Sei::Render { struct RenderQueue; }

namespace Sei::HUD
{
    bool Initialize(Render::RenderQueue& renderQueue);
    void Shutdown();

    // The section and its value strings must outlive their registration.
    void BindSection(const Section& section);
    void Submit();
    bool Draw(const std::vector<const Section*>& drawSections);
}
