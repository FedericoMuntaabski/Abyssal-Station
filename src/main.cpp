#include "core/Game.h"
#include <iostream>

int main()
{
    try {
        Game game(1280, 720, "Abyssal Station - Test");
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
