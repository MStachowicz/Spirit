#pragma once

#include "Platform/Input.hpp"

#include <functional>

namespace Platform
{
    class Input;
}
namespace ECS
{
    class Entity;
    class Storage;
}

namespace Component
{
    using InputFunction = std::function<void(const ECS::Entity&, ECS::Storage&, Platform::Input&)>;

    // Input component lets you attach a function to be called on every InputSystem::update.
    // The InputFunction has access to the Entity owener, the Storage it's contained in and the Input state.
    // Input also exposes some commonly-used functions that can be given at construction e.g. Input::Move.
    class Input
    {
    public:
        static InputFunction Move; // Basic move functionality. Depends on Entity owning a Transform component.

        Input(const InputFunction& p_input_func) noexcept
            : m_function{p_input_func}
        {}

        InputFunction m_function;
    };
} // namespace Component