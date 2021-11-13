#include "Input.hpp"
#include "Logger.hpp"
#include "Context.hpp"

 void Input::initialise(Context* pContext)
 {
     mGraphicsContext = pContext;
 }

void Input::onInput(const InputType &pInput)
{
    switch (pInput)
    {
    case InputType::Key_Escape:
        mGraphicsContext->close();
        break;
    case InputType::Unknown:
    default:
        LOG_WARN("Unknown key pressed passed to input library");
        break;
    }
}