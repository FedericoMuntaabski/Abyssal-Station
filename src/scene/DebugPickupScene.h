#ifndef ABYSSAL_STATION_SRC_SCENE_DEBUGPICKUPSCENE_H
#define ABYSSAL_STATION_SRC_SCENE_DEBUGPICKUPSCENE_H

#include "Scene.h"
#include <SFML/Graphics.hpp>

namespace scene { class SceneManager; }

namespace scene {

class DebugPickupScene : public Scene {
public:
    explicit DebugPickupScene(SceneManager* manager);
    ~DebugPickupScene() override = default;

    void handleEvent(sf::Event& event) override {}
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    void onEnter() override;
    void onExit() override;

private:
    SceneManager* m_manager{nullptr};
    sf::RectangleShape m_rect;
    float m_speed{60.f};
    float m_elapsed{0.f};
};

} // namespace scene

#endif // ABYSSAL_STATION_SRC_SCENE_DEBUGPICKUPSCENE_H
