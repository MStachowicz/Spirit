#include "Logger.hpp"
#include "Application.hpp"

int main(int argc, char *argv[])
{
    Logger::init(); // Logger is fully static and requires initialisation before any access.

    Application app(argc, argv);
    if(app.initialise())
        app.simulationLoop();
    else
        return EXIT_FAILURE;

    LOG_INFO("Exit requested, closing application.");
    return EXIT_SUCCESS;
}