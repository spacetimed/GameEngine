#include "Keybinds.h"
#include "Input.h"

#include <unordered_map>
#include <utility>

namespace
{
    std::unordered_map<unsigned int, std::function<void()>> bindings;
}

namespace Sei::Keybinds
{
    void Bind(unsigned int key, std::function<void()> action)
    {
        bindings[key] = std::move(action);
    }

    void Update()
    {
        for (const auto& [key, action] : bindings)
            if (action && Input::KeyPressed(key)) action();
    }

    void Clear()
    {
        bindings.clear();
    }
}
