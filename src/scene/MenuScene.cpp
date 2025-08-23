#include "MenuScene.h"
#include "SceneManager.h"
#include "PlayScene.h"
#include "../core/Logger.h"
#include "../core/FontHelper.h"

#include <utility>
#include <filesystem>
#include <cstdlib>
#include "../ui/UIManager.h"
#include "../ui/MainMenu.h"

namespace scene {

using core::Logger;

MenuScene::MenuScene(SceneManager* manager)
    : m_manager(manager) {
}

void MenuScene::onEnter() {
    // Try to locate a font file by searching upwards from the current working directory.
    namespace fs = std::filesystem;

    auto findUp = [&](const std::string& rel) -> std::string {
        fs::path p = fs::current_path();
        for (int i = 0; i < 8; ++i) {
            fs::path cand = p / rel;
            if (fs::exists(cand) && fs::is_regular_file(cand)) return cand.string();
            if (p.has_parent_path()) p = p.parent_path(); else break;
        }
        return std::string();
    };

    // Use common helper to find and load a font
    if (core::loadBestFont(m_font)) {
        m_fontLoaded = true;
        m_text = std::make_unique<sf::Text>(m_font, "Presione Enter para jugar", 24);
        m_text->setFillColor(sf::Color::White);
        m_text->setPosition(sf::Vector2f(100.f, 100.f));
    } else {
        Logger::instance().warning("MenuScene: failed to load a font via FontHelper");
    }

    Logger::instance().info("MenuScene: onEnter");

    // Create UIManager and push the main menu
    m_uiManager = std::make_unique<ui::UIManager>();
    m_uiManager->pushMenu(new ui::MainMenu(m_manager, m_uiManager.get()));
}

void MenuScene::onExit() {
    Logger::instance().info("MenuScene: onExit");
}

void MenuScene::handleEvent(sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>()) {
        if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
            if (kp->code == sf::Keyboard::Key::Enter) {
                Logger::instance().info("MenuScene: Enter pressed -> replacing with PlayScene");
                if (m_manager) m_manager->replace(std::make_unique<PlayScene>(m_manager));
            } else if (kp->code == sf::Keyboard::Key::Escape) {
                Logger::instance().info("MenuScene: Escape pressed -> popping scene");
                if (m_manager) m_manager->pop();
            }
        }
    }
}

void MenuScene::update(float dt) {
    // no-op for menu
    (void)dt;
    if (m_uiManager) m_uiManager->update(dt);
}

void MenuScene::render(sf::RenderWindow& window) {
    if (m_text) window.draw(*m_text);
    if (m_uiManager) m_uiManager->render(window);
}

} // namespace scene
