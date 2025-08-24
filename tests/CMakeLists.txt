cmake_minimum_required(VERSION 3.10)

# Find Google Test
find_package(GTest CONFIG REQUIRED)
if(NOT GTest_FOUND)
    message(STATUS "Google Test not found. Install via vcpkg: vcpkg install gtest")
    message(FATAL_ERROR "Google Test is required for unit tests.")
endif()

# Find SFML (already found in main CMakeLists.txt)
find_package(SFML COMPONENTS Graphics Window System Audio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Create test executable
add_executable(InputManagerTests
    main.cpp
    input/InputManagerTest.cpp
    # Add the actual source files we're testing
    ../src/input/InputManager.cpp
    ../src/input/CommandSystem.cpp
    ../src/core/Logger.cpp
)

# Include directories
target_include_directories(InputManagerTests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Link libraries
target_link_libraries(InputManagerTests PRIVATE 
    GTest::gtest
    GTest::gtest_main
    SFML::Graphics 
    SFML::Window 
    SFML::System 
    SFML::Audio
    nlohmann_json::nlohmann_json
)

# Set C++ standard
set_target_properties(InputManagerTests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(InputManagerTests)

# Add Gameplay tests
add_executable(GameplayTests
    gameplay/simple_main.cpp
    gameplay/stubs.cpp
    # Add the actual source files we're testing
    ../src/gameplay/Item.cpp
    ../src/gameplay/ItemManager.cpp
    ../src/gameplay/Puzzle.cpp
    ../src/gameplay/PuzzleManager.cpp
    ../src/entities/Entity.cpp
    ../src/collisions/CollisionManager.cpp
    ../src/collisions/CollisionBox.cpp
    ../src/collisions/SpatialPartition.cpp
    ../src/core/Logger.cpp
)

# Include directories for gameplay tests
target_include_directories(GameplayTests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Link libraries for gameplay tests  
target_link_libraries(GameplayTests PRIVATE 
    SFML::Graphics 
    SFML::Window 
    SFML::System 
    SFML::Audio
)

# Set C++ standard for gameplay tests
set_target_properties(GameplayTests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add Entity tests
add_executable(EntityTests
    entities/EntityTests.cpp
    # Add the actual source files we're testing
    ../src/entities/Entity.cpp
    ../src/entities/EntityManager.cpp
    ../src/entities/Player.cpp
    ../src/entities/MovementHelper.cpp
    # ../src/entities/EntityTelemetry.cpp  # Disabled due to SFML compatibility
    ../src/entities/EntityDebug.cpp
    ../src/collisions/CollisionManager.cpp
    ../src/collisions/CollisionBox.cpp
    ../src/collisions/SpatialPartition.cpp
    ../src/core/Logger.cpp
    ../src/core/GameState.h
    ../src/scene/SaveIntegration.cpp
    ../src/gameplay/ItemManager.cpp
    ../src/gameplay/PuzzleManager.cpp
    ../src/gameplay/Item.cpp
    ../src/gameplay/Puzzle.cpp
    ../src/input/InputManager.cpp
    ../src/ui/UIManager.cpp
    ../src/ui/Menu.cpp
    ../src/ui/CustomCursor.cpp
    ../src/core/AssetManager.cpp
    ../src/core/ConfigManager.cpp
)

# Include directories for entity tests
target_include_directories(EntityTests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Link libraries for entity tests
target_link_libraries(EntityTests PRIVATE 
    SFML::Graphics 
    SFML::Window 
    SFML::System 
    SFML::Audio
    nlohmann_json::nlohmann_json
)

# Set C++ standard for entity tests
set_target_properties(EntityTests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add Collision tests
add_executable(CollisionTests
    collisions/CollisionTests.cpp
    main.cpp
    # Add the actual source files we're testing
    ../src/collisions/CollisionBox.cpp
    ../src/collisions/CollisionManager.cpp
    ../src/collisions/CollisionSystem.cpp
    ../src/collisions/CollisionEvents.cpp
    ../src/collisions/SpatialPartition.cpp
    ../src/collisions/CollisionDebug.cpp
    ../src/entities/Entity.cpp
    ../src/core/Logger.cpp
)

# Include directories for collision tests
target_include_directories(CollisionTests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Link libraries for collision tests
target_link_libraries(CollisionTests PRIVATE 
    GTest::gtest
    GTest::gtest_main
    SFML::Graphics 
    SFML::Window 
    SFML::System 
    SFML::Audio
)

# Set C++ standard for collision tests
set_target_properties(CollisionTests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Register collision tests with CTest
gtest_discover_tests(CollisionTests)

# Create AI tests executable
add_executable(AITests
    ai/AITests.cpp
    # Add the actual source files we're testing
    ../src/ai/AIState.cpp
    ../src/ai/Perception.cpp
    ../src/ai/Pathfinding.cpp
    ../src/ai/AISystem.cpp
    ../src/ai/AIManager.cpp
    ../src/ai/Enemy.cpp
    # ../src/ai/BehaviorStrategy.cpp
    ../src/entities/Entity.cpp
    ../src/entities/Player.cpp
    ../src/entities/EntityManager.cpp
    ../src/entities/MovementHelper.cpp
    ../src/collisions/CollisionManager.cpp
    ../src/collisions/CollisionBox.cpp
    ../src/collisions/CollisionSystem.cpp
    ../src/collisions/CollisionEvents.cpp
    ../src/collisions/SpatialPartition.cpp
    ../src/input/InputManager.cpp
    ../src/core/Logger.cpp
)

# Include directories for AI tests
target_include_directories(AITests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Link libraries for AI tests
target_link_libraries(AITests PRIVATE 
    GTest::gtest 
    GTest::gtest_main
    SFML::Graphics 
    SFML::Window 
    SFML::System
    SFML::Audio
    nlohmann_json::nlohmann_json
)

# Set C++ standard for AI tests
set_target_properties(AITests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Register AI tests with CTest
gtest_discover_tests(AITests)

# Create UI/Scene Navigation tests executable
add_executable(SceneNavigationTests
    ui/SceneNavigationTests.cpp
    main.cpp
    # Add the actual source files we're testing
    ../src/ui/MainMenu.cpp
    ../src/ui/OptionsMenu.cpp
    ../src/ui/PauseMenu.cpp
    ../src/ui/UIManager.cpp
    ../src/ui/Menu.cpp
    ../src/ui/InputHelper.cpp
    ../src/ui/CustomCursor.cpp
    ../src/core/AssetManager.cpp
    ../src/core/ConfigManager.cpp
    ../src/scene/SceneManager.cpp
    ../src/scene/MenuScene.cpp
    ../src/scene/PlayScene.cpp
    ../src/input/InputManager.cpp
    ../src/core/Logger.cpp
    ../src/core/FontHelper.cpp
    ../src/entities/EntityManager.cpp
    ../src/entities/Entity.cpp
    ../src/entities/Player.cpp
    ../src/entities/Wall.cpp
    ../src/entities/EntityFactory.cpp
    ../src/ai/Enemy.cpp
    ../src/ai/EnemyManager.cpp
    ../src/ai/AIState.cpp
    ../src/ai/Perception.cpp
    ../src/ai/Pathfinding.cpp
    ../src/ai/AISystem.cpp
    ../src/ai/AIManager.cpp
    # ../src/ai/BehaviorStrategy.cpp
    ../src/entities/MovementHelper.cpp
    ../src/collisions/CollisionManager.cpp
    ../src/collisions/CollisionBox.cpp
    ../src/collisions/CollisionSystem.cpp
    ../src/collisions/CollisionEvents.cpp
    ../src/collisions/SpatialPartition.cpp
    ../src/collisions/CollisionDebug.cpp
    ../src/gameplay/ItemManager.cpp
    ../src/gameplay/PuzzleManager.cpp
    ../src/gameplay/Item.cpp
    ../src/gameplay/Puzzle.cpp
)

# Include directories for scene navigation tests
target_include_directories(SceneNavigationTests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Link libraries for scene navigation tests
target_link_libraries(SceneNavigationTests PRIVATE 
    GTest::gtest 
    GTest::gtest_main
    SFML::Graphics 
    SFML::Window 
    SFML::System
    SFML::Audio
    nlohmann_json::nlohmann_json
)

# Set C++ standard for scene navigation tests
set_target_properties(SceneNavigationTests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Register scene navigation tests with CTest
gtest_discover_tests(SceneNavigationTests)
