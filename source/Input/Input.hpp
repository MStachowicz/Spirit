#pragma once

#include "InputAPI.hpp"

// Processes input coming in from a registered mInputHandler using the InputAPI.
class Input
{
public:
    void initialise();
    void pollEvents();
    bool closeRequested();
private:
    void onInput(const InputAPI::Key& pKeyPressed);
    InputAPI *mInputHandler = nullptr;
    bool mCloseRequested = false;
};