#ifndef A6C1D2_PLAYSCENE_H
#define A6C1D2_PLAYSCENE_H

#include "Scene.h"
#include <SFML/Graphics.hpp>
#include "../ai/EnemyManager.h"
#include "../gameplay/ItemManager.h"
#include "../gameplay/PuzzleManager.h"
#include "../gameplay/NoiseSystem.h"
#include "../gameplay/VisionSystem.h"
#include "../core/SaveManager.h"
#include "../debug/DebugConsole.h"
#include "../debug/DebugOverlay.h"
#include "../ui/SurvivalHUD.h"

namespace scene { class SceneManager; }
namespace ui { class UIManager; class PauseMenu; }

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
    std::unique_ptr<ui::UIManager> m_uiManager;
    std::unique_ptr<ui::PauseMenu> m_pauseMenu;
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
    // Gameplay managers
    std::unique_ptr<gameplay::ItemManager> m_itemManager;
    std::unique_ptr<gameplay::PuzzleManager> m_puzzleManager;
    
    // NEW: Advanced gameplay systems  
    std::unique_ptr<gameplay::NoiseSystem> m_noiseSystem;
    std::unique_ptr<gameplay::VisionSystem> m_visionSystem;
    
    // Auto-save system
    std::unique_ptr<core::SaveManager> m_saveManager;
    
    // Debug systems
    std::unique_ptr<debug::DebugConsole> m_debugConsole;
    std::unique_ptr<debug::DebugOverlay> m_debugOverlay;
    
    // NEW: Survival HUD system
    std::unique_ptr<ui::SurvivalHUD> m_survivalHUD;
    
    // Achievement tracking
    float m_survivalTime{0.0f};
    
    // Interaction hint state
    bool m_showInteractHint{false};
    uint32_t m_nearbyItemId{0u};
    sf::Font m_hintFont;
    // icon for interaction hint (optional)
    sf::Texture m_hintTexture;
    std::unique_ptr<sf::Sprite> m_hintSprite;
    float m_hintPulseTimer{0.f};
    std::unique_ptr<sf::Text> m_hintText; // kept as fallback if icon missing
    
    // Helper methods
    std::string formatFloat(float value, int precision) const;
};

} // namespace scene

#endif // A6C1D2_PLAYSCENE_H
