#include "MenuScene.h"
#include "SceneManager.h"
#include "PlayScene.h"
#include "LoadingScene.h"
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
    
    // Set up UI event callbacks for proper scene transitions
    ui::UIEvents events;
    events.onStartGame = [this]() {
        Logger::instance().info("MenuScene: Starting game - transitioning to LoadingScene");
        if (m_manager) {
            // Create PlayScene as the target scene for LoadingScene
            auto playScene = std::make_unique<scene::PlayScene>(m_manager);
            // Push LoadingScene which will transition to PlayScene after 5 seconds
            m_manager->push(std::make_unique<scene::LoadingScene>(std::move(playScene), m_manager));
        }
    };
    events.onExit = [this]() {
        Logger::instance().info("MenuScene: Exit requested");
        if (m_manager) {
            m_manager->pop();
        }
    };
    m_uiManager->setEventCallbacks(events);
    
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
