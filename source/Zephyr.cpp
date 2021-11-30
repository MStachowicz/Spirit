#include <cstdlib>
#include "Logger.hpp"
#include "JobSystem.hpp"
#include "Renderer.hpp"
#include "FileSystem.hpp"

void processArguments(int argc, char *argv[])
{
    LOG_INFO("Number of arguments passed on launch: {}", argc);
    for (int index{}; index != argc; ++index)
        LOG_INFO("Argument {}: {}", index + 1, argv[index]);

    ZEPHYR_ASSERT(argc > 0, "No arguments supplied to executable, directories cannot be initialised")
    File::setupDirectories(argv[0]);
}

int main(int argc, char *argv[])
{
    Logger::init();

    processArguments(argc, argv);

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