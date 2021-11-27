#include <cstdlib>
#include "Logger.hpp"
#include "JobSystem.hpp"
#include "Renderer.hpp"


void printArguments(int argc, char *argv[])
{
    LOG_INFO("Number of arguments passed on launch: {}", argc);
    for (int index{}; index != argc; ++index)
        LOG_INFO("Argument {}: {}", index + 1, argv[index]);
}

int main(int argc, char *argv[])
{
    Logger::init();

    printArguments(argc, argv);

    JobSystem::initialise();

    Renderer renderer;
    if (!renderer.initialise())
    {
        LOG_CRITICAL("Failed to initalise Renderer");
        return EXIT_FAILURE;
    }

    // Continuous loop until the main window is marked for closing
    renderer.drawLoop();

    return EXIT_SUCCESS;
}