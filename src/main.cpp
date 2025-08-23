#include "core/Game.h"
#include <iostream>
#include "core/Logger.h"
#include <csignal>

static void myTerminateHandler() {
    try { core::Logger::instance().error("Program terminating via std::terminate"); } catch(...) {}
    std::abort();
}

int main()
{
    std::set_terminate(myTerminateHandler);
    try {
        Game game(1280, 720, "Abyssal Station - Test");
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
