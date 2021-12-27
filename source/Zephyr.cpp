#include "Logger.hpp"
#include "Application.hpp"

int main(int argc, char *argv[])
{
    {
        Application app;
        if (app.initialise(argc, argv))
            app.simulationLoop();
        else
            return EXIT_FAILURE;
    }

    LOG_INFO("Exit requested, closing application.");
    return EXIT_SUCCESS;
}