#ifndef A6C1D2_PLAYSCENE_H
#define A6C1D2_PLAYSCENE_H

#include "Scene.h"
#include <SFML/Graphics.hpp>
#include "../ai/EnemyManager.h"

namespace scene { class SceneManager; }

namespace entities { class Player; class EntityManager; class Wall; }
namespace collisions { class CollisionManager; class CollisionSystem; }

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
    // Entities manager for this scene
    std::unique_ptr<entities::EntityManager> m_entityManager;
    std::unique_ptr<collisions::CollisionManager> m_collisionManager;
    std::unique_ptr<collisions::CollisionSystem> m_collisionSystem;
    // keep a raw pointer to player if needed
    entities::Player* m_player{nullptr};
    // Enemy manager for AI planning
    std::unique_ptr<ai::EnemyManager> m_enemyManager;
};

} // namespace scene

#endif // A6C1D2_PLAYSCENE_H
