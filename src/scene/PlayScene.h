#ifndef A6C1D2_PLAYSCENE_H
#define A6C1D2_PLAYSCENE_H

#include "Scene.h"
#include <SFML/Graphics.hpp>

namespace scene { class SceneManager; }

namespace scene {

class PlayScene : public Scene {
public:
    explicit PlayScene(SceneManager* manager);
    ~PlayScene() override = default;

    void handleEvent(sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    void onEnter() override;
    void onExit() override;

private:
    SceneManager* m_manager{nullptr};
    sf::RectangleShape m_rect;
    sf::Vector2f m_velocity{0.f, 0.f};
    float m_speed{200.f};
};

} // namespace scene

#endif // A6C1D2_PLAYSCENE_H
