#pragma once

class Context;

// This is a fully static input class allowing it to be linked to GLFW's input callbacks
// which require a global or static implementation. The linkedGraphicsContext is the 
// graphics API and window context the inputs will affect.
class Input
{
public:
    static void onInput(const int& pInputKey);
    // The linked graphics context this input manager will manipulate.
    static Context* linkedGraphicsContext; 
};