#include "core/Game.h"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#include <clocale>
#endif

int main()
{
    try {
#ifdef _WIN32
    // Set console to UTF-8 to properly display accented characters in logs
    SetConsoleOutputCP(CP_UTF8);
    // Set C locale to user's default which, combined with UTF-8 CP, improves accent rendering
    std::setlocale(LC_ALL, "");
#endif
        // Default resolution set to 1920x1080 as requested
        Game game(1920, 1080, "Abyssal Station - Test");
    std::ios_base::sync_with_stdio(true);
    game.run();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
