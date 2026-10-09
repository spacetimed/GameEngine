#pragma once

#include <functional>

namespace Sei::Keybinds
{
    void Bind(unsigned int key, std::function<void()> action);
    void Update(); // Call after Input::Update().
    void Clear();
}
