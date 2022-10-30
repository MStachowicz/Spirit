#include "Application.hpp"

#include "File.hpp"
#include "JobSystem.hpp"
#include "Logger.hpp"
#include "Stopwatch.hpp"

int main(int argc, char *argv[])
{
    Utility::Stopwatch stopwatch;

    Logger::initialise(); // Logger must be initialised first as everything depends on it for logging and error checks.
    ZEPHYR_ASSERT(argc > 0, "No arguments supplied to executable, directories cannot be initialised")

    LOG_INFO("Number of arguments passed on launch: {}", argc);
    for (int index{}; index != argc; ++index)
        LOG_INFO("Argument {}: {}", index + 1, argv[index]);

    Utility::File::setupDirectories(argv[0]);
    JobSystem::initialise();

    Application app;
    LOG_INFO("Zephyr initialisation took {0:.3f}ms", stopwatch.getTime<std::ratio<1>, float>());

    app.simulationLoop();

    LOG_INFO("Exit requested, closing application.");
    return EXIT_SUCCESS;
}