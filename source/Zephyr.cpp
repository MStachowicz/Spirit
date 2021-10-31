#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "Renderer.hpp"
#include "Logger.hpp"

int main(int argc, char* argv[])
{
    std::cout << "argc == " << argc << '\n';

    for (int ndx{}; ndx != argc; ++ndx) {
        std::cout << "argv[" << ndx << "] == " << std::quoted(argv[ndx]) << '\n';
    }
    std::cout << "argv[" << argc << "] == "
        << static_cast<void*>(argv[argc]) << '\n';
    /*...*/

    Logger::init();
	Renderer::init();

	return argc == 3 ? EXIT_SUCCESS : EXIT_FAILURE; // optional return value
}