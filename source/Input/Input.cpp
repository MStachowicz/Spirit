#include "Input.hpp"
#include "Logger.hpp"
#include "Context.hpp"

Context* Input::linkedGraphicsContext = nullptr; 

void Input::onInput(const InputType &pInput)
{
    switch (pInput)
    {
    case InputType::Key_Escape:
        linkedGraphicsContext->close();
        break;
    case InputType::Unknown:
    default:
        LOG_WARN("Unknown key pressed passed to input library");
        break;
    }
}