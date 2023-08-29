#pragma once

#include "Platform/Input.hpp"
#include "Utility/Config.hpp"

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
    using InputFunction = std::function<void(const DeltaTime& p_delta_time, const ECS::Entity&, ECS::Storage&, Platform::Input&)>;

    // Input component lets you attach a function to be called on every InputSystem::update.
    // The InputFunction has access to the Entity owener, the Storage it's contained in and the Input state.
    // Input also exposes some commonly-used functions that can be given at construction e.g. Input::Move.
    class Input
    {
    public:
        static InputFunction Camera_Move_Look; // Basic move functionality. Depends on Entity owning Camera+Transform+RigidBody components.

        Input(const InputFunction& p_input_func) noexcept
            : m_function{p_input_func}
        {}

        InputFunction m_function;
    };
} // namespace Component