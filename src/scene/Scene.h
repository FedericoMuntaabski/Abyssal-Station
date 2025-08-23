#ifndef A6C1D2_SCENE_H
#define A6C1D2_SCENE_H

#include <SFML/Graphics.hpp>

namespace scene {

// Abstract base class for all scenes/screens in the game.
class Scene {
public:
    // Virtual destructor for proper cleanup in derived classes.
    virtual ~Scene() = default;

    // Pure virtual interface methods that concrete scenes must implement.
    virtual void handleEvent(sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;

    // Optional lifecycle hooks.
    virtual void onEnter() {}
    virtual void onExit() {}
};

} // namespace scene

#endif // A6C1D2_SCENE_H
