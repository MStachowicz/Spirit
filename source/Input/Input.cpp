#include "Input.hpp"
#include "Logger.hpp"
#include "Context.hpp"

// ZEPHYR_KEY values match the GLFW_KEY values in glfw3.h
#define ZEPHYR_KEY_ESCAPE   256
#define ZEPHYR_KEY_ENTER    257
#define ZEPHYR_KEY_UNKNOWN  -1

Context* Input::linkedGraphicsContext = nullptr;

void Input::pollEvents()
{
    linkedGraphicsContext->pollEvents();
}

// This function is linked to the GLFW key callback
void Input::onInput(const int& pInputKey)
{
    switch (pInputKey)
    {
    case ZEPHYR_KEY_ESCAPE:
        linkedGraphicsContext->close();
        break;
    case ZEPHYR_KEY_ENTER:
        linkedGraphicsContext->setClearColour(255,255,255);
        break;
    case ZEPHYR_KEY_UNKNOWN:
    default:
        LOG_WARN("Unknown key press of value {}", pInputKey);
        break;
    }
}

bool Input::closeRequested()
{
    return linkedGraphicsContext->isClosing();
}