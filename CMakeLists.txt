cmake_minimum_required(VERSION 3.10)
project(AbyssalStation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If user passed a toolchain (for example vcpkg), CMake will use it. This
# project expects SFML to be available via find_package. Recommend using vcpkg
# if SFML isn't installed system-wide.

message(STATUS "CMake generator: ${CMAKE_GENERATOR}")
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

## Prefer the imported config targets provided by modern SFML packages (vcpkg installs SFML 3.x)
find_package(SFML COMPONENTS Graphics Window System Audio CONFIG REQUIRED)
if(NOT SFML_FOUND)
    message(STATUS "SFML not found by CMake. If you installed SFML via vcpkg, run CMake with -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake")
    message(FATAL_ERROR "SFML (graphics, window, system) is required. Set SFML_DIR or use vcpkg.")
endif()

add_executable(AbyssalStation
    src/main.cpp
    src/core/Game.cpp
    src/core/Game.h
    src/core/AssetManager.cpp
    src/core/Logger.cpp
    src/core/FontHelper.cpp
    src/core/Timer.cpp
    # Scene module sources
    src/scene/SceneManager.cpp
    src/scene/MenuScene.cpp
    src/scene/PlayScene.cpp
    src/scene/Scene.h
    src/scene/SceneManager.h
    src/scene/MenuScene.h
    src/scene/PlayScene.h
    # Input module
    src/input/InputManager.cpp
    src/input/InputManager.h
    # Entities
    src/entities/Entity.cpp
    src/entities/Entity.h
    src/entities/Player.cpp
    src/entities/Player.h
    src/entities/Wall.cpp
    src/entities/Wall.h
    src/entities/EntityManager.cpp
    src/entities/EntityManager.h

    # AI module
    src/ai/Enemy.cpp
    src/ai/Enemy.h
    src/ai/EnemyManager.cpp
    src/ai/EnemyManager.h

    # Collisions module
    src/collisions/CollisionBox.cpp
    src/collisions/CollisionManager.cpp
    src/collisions/CollisionSystem.cpp
    # Gameplay: items & puzzles
    src/gameplay/Item.cpp
    src/gameplay/ItemManager.cpp
    src/gameplay/Puzzle.cpp
    src/gameplay/PuzzleManager.cpp
    # Debug scene for pickup test
    src/scene/DebugPickupScene.cpp
    # UI module
    src/ui/Menu.cpp
    src/ui/MainMenu.cpp
    src/ui/PauseMenu.cpp
    src/ui/OptionsMenu.cpp
    src/ui/UIManager.cpp
)

target_include_directories(AbyssalStation PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(AbyssalStation PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)

# Copy assets folder to the target directory after build so the executable can load resources using relative paths
add_custom_command(TARGET AbyssalStation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:AbyssalStation>/assets
)
