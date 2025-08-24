#include "core/Game.h"
#include <iostream>

int main()
{
    try {
    // Default resolution set to 1920x1080 as requested
    Game game(1920, 1080, "Abyssal Station - Test");
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
