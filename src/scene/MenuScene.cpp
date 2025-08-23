#include "MenuScene.h"
#include "SceneManager.h"
#include "PlayScene.h"
#include "../core/Logger.h"

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

    std::string fontPath;
    // Common locations
    fontPath = findUp("assets/fonts/arial.ttf");
    if (fontPath.empty()) fontPath = findUp("assets/textures/arial.ttf");

    // Try any ttf/otf in assets/fonts
    if (fontPath.empty()) {
        fs::path p = fs::current_path();
        for (int i = 0; i < 8 && !fontPath.size(); ++i) {
            fs::path candDir = p / "assets" / "fonts";
            if (fs::exists(candDir) && fs::is_directory(candDir)) {
                for (auto &entry : fs::directory_iterator(candDir)) {
                    if (!entry.is_regular_file()) continue;
                    auto ext = entry.path().extension().string();
                    if (ext == ".ttf" || ext == ".otf") { fontPath = entry.path().string(); break; }
                }
            }
            if (p.has_parent_path()) p = p.parent_path(); else break;
        }
    }

#ifdef _WIN32
    if (fontPath.empty()) {
        // Use _dupenv_s to avoid MSVC deprecation warning for getenv
        char* windir = nullptr;
        size_t len = 0;
        if (_dupenv_s(&windir, &len, "WINDIR") == 0 && windir) {
            fs::path winFont = fs::path(windir) / "Fonts" / "arial.ttf";
            if (fs::exists(winFont)) fontPath = winFont.string();
        }
        if (windir) free(windir);
    }
#endif

    if (!fontPath.empty()) {
        Logger::instance().info(std::string("MenuScene: attempting to open font: ") + fontPath);
        if (m_font.openFromFile(fontPath)) {
            m_fontLoaded = true;
            m_text = std::make_unique<sf::Text>(m_font, "Presione Enter para jugar", 24);
            m_text->setFillColor(sf::Color::White);
            m_text->setPosition(sf::Vector2f(100.f, 100.f));
        } else {
            Logger::instance().warning(std::string("MenuScene: failed to open font file: ") + fontPath);
        }
    } else {
        Logger::instance().warning("MenuScene: no font file found (looked for assets/fonts/*.ttf, assets/textures/arial.ttf, and Windows fonts)");
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
