#include "Logger.hpp"
#include "Application.hpp"
#include "JobSystem.hpp"
#include "FileSystem.hpp"
#include "Utility.hpp"

int main(int argc, char *argv[])
{
    Application::Clock::time_point initStartTime = Application::Clock::now();

    Logger::initialise(); // Logger must be initialised first as everything depends on it for logging and error checks.
    ZEPHYR_ASSERT(argc > 0, "No arguments supplied to executable, directories cannot be initialised")

    LOG_INFO("Number of arguments passed on launch: {}", argc);
    for (int index{}; index != argc; ++index)
        LOG_INFO("Argument {}: {}", index + 1, argv[index]);

    File::setupDirectories(argv[0]);
    util::File::initialise(argv[0]);
    JobSystem::initialise();

    Application app;
    LOG_INFO("Zephyr initialisation took {}ms", std::chrono::round<std::chrono::milliseconds>(Application::Clock::now() - initStartTime).count());

    app.simulationLoop();

    LOG_INFO("Exit requested, closing application.");
    return EXIT_SUCCESS;
}