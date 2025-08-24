#ifndef A6C1D2_MENUSCENE_H
#define A6C1D2_MENUSCENE_H

#include "Scene.h"
#include <SFML/Graphics.hpp>
#include <memory>
namespace scene { class SceneManager; class PlayScene; }
namespace ui { class UIManager; }

namespace scene {

class MenuScene : public Scene {
public:
    explicit MenuScene(SceneManager* manager, core::ConfigManager* cfg = nullptr);
    ~MenuScene() override = default;

    void handleEvent(sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    void onEnter() override;
    void onExit() override;

private:
    SceneManager* m_manager{nullptr};
    std::unique_ptr<ui::UIManager> m_uiManager;
    core::ConfigManager* m_configManager{nullptr};
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_text;
    bool m_fontLoaded{false};
};

} // namespace scene

#endif // A6C1D2_MENUSCENE_H
