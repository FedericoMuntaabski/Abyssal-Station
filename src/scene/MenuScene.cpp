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

MenuScene::MenuScene(SceneManager* manager, core::ConfigManager* cfg)
    : m_manager(manager), m_configManager(cfg) {
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

    // Use common helper to find and load a font (keep for UI usage, but do not create
    // the legacy "Presione Enter para jugar" prompt; main menu handles selection now).
    if (core::loadBestFont(m_font)) {
        m_fontLoaded = true;
        // Intentionally do not create the old prompt text here.
    } else {
        Logger::instance().warning("MenuScene: failed to load a font via FontHelper");
    }

    Logger::instance().info("MenuScene: onEnter");

    // Create UIManager and push the main menu
    m_uiManager = std::make_unique<ui::UIManager>();
    // If a ConfigManager was provided, register it with the UIManager so menus can access persisted settings
    if (m_configManager) {
        m_uiManager->setConfigManager(m_configManager);
        // Apply config to UI (notification durations, etc.)
        m_configManager->applyConfig(*m_uiManager);
    }
    m_uiManager->pushMenu(new ui::MainMenu(m_manager, m_uiManager.get()));
}

void MenuScene::onExit() {
    Logger::instance().info("MenuScene: onExit");
}

void MenuScene::handleEvent(sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>()) {
        if (auto kp = event.getIf<sf::Event::KeyPressed>()) {
            // Do NOT auto-start the game on Enter here; the UI/MainMenu handles selection.
            if (kp->code == sf::Keyboard::Key::Escape) {
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
    // No legacy prompt text is drawn here. UIManager/MainMenu renders the menu and hints.
    if (m_uiManager) m_uiManager->render(window);
}

} // namespace scene
