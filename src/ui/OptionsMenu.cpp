#include "OptionsMenu.h"
#include "../input/InputManager.h"
#include "../core/Logger.h"
#include "../core/ConfigManager.h"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>

namespace ui {

using input::InputManager;

OptionsMenu::OptionsMenu(scene::SceneManager* manager)
    : Menu("OptionsMenu"), m_manager(manager) {
    m_options = {"Volume", "Controls", "Back"};
    try {
        core::Logger::instance().info("OptionsMenu: constructed; manager=" + std::string(m_manager ? "valid" : "null"));
    } catch(...) {}
}

void OptionsMenu::onEnter() {
    try {
        core::Logger::instance().info("OptionsMenu: onEnter");
    } catch(...) {}
    m_selected = 0;
}

void OptionsMenu::onExit() {}

int OptionsMenu::volume() const noexcept { return m_volume; }

void OptionsMenu::setVolume(int vol) {
    m_volume = std::clamp(vol, 0, 100);
    applyVolume();
    core::ConfigManager cfg;
    if (cfg.loadConfig()) {
        cfg.setVolumeValue(m_volume);
        cfg.saveConfig();
        // apply to UI manager if available (OptionsMenu could have access to UI via SceneManager)
    }
}

void OptionsMenu::applyVolume() {
    core::Logger::instance().info(std::string("Volume set to ") + std::to_string(m_volume));
    // TODO: integrate with audio system
}

void OptionsMenu::handleInput() {
    try {
        core::Logger::instance().info("OptionsMenu: handleInput start");
    } catch(...) {}
    try {
        auto& im = InputManager::getInstance();

    if (im.isActionJustPressed(input::Action::MoveUp)) {
        if (m_selected == 0) m_selected = m_options.size() - 1;
        else --m_selected;
    }

    if (im.isActionJustPressed(input::Action::MoveDown)) {
        m_selected = (m_selected + 1) % m_options.size();
    }

    // Adjust volume with left/right when Volume option selected
    if (m_selected == 0) {
        if (im.isActionJustPressed(input::Action::MoveLeft)) {
            setVolume(m_volume - 5);
        }
        if (im.isActionJustPressed(input::Action::MoveRight)) {
            setVolume(m_volume + 5);
        }
    }

    if (im.isActionJustPressed(input::Action::Confirm)) {
        const auto& choice = m_options[m_selected];
        core::Logger::instance().info("OptionsMenu selected: " + choice);
        if (choice == "Back") {
            deactivate();
        } else if (choice == "Controls") {
            // enter controls panel; by default select first action
            m_selected = 1; // Controls index
        }
        // If we're in the controls panel and user confirms on a listed action, start remap
        if (m_selected == 1) {
            // calculate which action row was clicked/selected by user: reuse m_options selection offset
            // show remap for the first action in our actions list if not already remapping
            if (!m_waitingForRemap) {
                m_waitingForRemap = true;
                m_actionToRemap = m_actionsToShow.front();
                core::Logger::instance().info("OptionsMenu: starting remap for " + std::string("action"));
            }
        }
    }
    } catch (const std::exception& e) {
        try { core::Logger::instance().error(std::string("OptionsMenu::handleInput exception: ") + e.what()); } catch(...) {}
    } catch (...) {
        try { core::Logger::instance().error("OptionsMenu::handleInput unknown exception"); } catch(...) {}
    }
}

void OptionsMenu::update(float dt) {
    try {
        core::Logger::instance().info("OptionsMenu: update start");
    } catch(...) {}
    (void)dt;
    try {
    if (m_waitingForRemap) {
        auto& im = InputManager::getInstance();
        auto lastKey = im.getLastKeyEvent();
        if (lastKey.first) {
            im.rebindKeys(m_actionToRemap, {lastKey.second});
            core::Logger::instance().info(std::string("Rebound action to key ") + std::to_string(static_cast<int>(lastKey.second)));
            m_waitingForRemap = false;
            im.clearLastEvents();
                    // Persist new bindings
                    core::ConfigManager cfg;
                    cfg.saveBindingsFromInput();
        } else {
            auto lastMouse = im.getLastMouseButtonEvent();
            if (lastMouse.first) {
                im.rebindMouse(m_actionToRemap, {lastMouse.second});
                core::Logger::instance().info(std::string("Rebound action to mouse button ") + std::to_string(static_cast<int>(lastMouse.second)));
                m_waitingForRemap = false;
                im.clearLastEvents();
                        // Persist new bindings
                        core::ConfigManager cfg;
                        cfg.saveBindingsFromInput();
            }
        }
    }
    } catch (const std::exception& e) {
        try { core::Logger::instance().error(std::string("OptionsMenu::update exception: ") + e.what()); } catch(...) {}
    } catch (...) {
        try { core::Logger::instance().error("OptionsMenu::update unknown exception"); } catch(...) {}
    }
}

void OptionsMenu::render(sf::RenderWindow& window) {
    try {
        core::Logger::instance().info("OptionsMenu: render start");
    } catch(...) {}
    static sf::Font font;
    static bool fontLoaded = false;
    if (!fontLoaded) {
        fontLoaded = font.openFromFile("assets/fonts/Long_Shot.ttf");
    }

    for (std::size_t i = 0; i < m_options.size(); ++i) {
        std::string content;
        if (i == 0) {
            // show volume value inline
            content = m_options[i] + std::string(": ") + std::to_string(m_volume);
        } else {
            content = m_options[i];
        }
        sf::Text text(font, content, 20);
        text.setFillColor(i == m_selected ? sf::Color::Green : sf::Color::White);
        text.setPosition(sf::Vector2f(110.f, m_startY + static_cast<float>(i) * m_spacing));
        window.draw(text);
    }

    // If showing Controls, also render current bindings and remap instructions
    if (true) { // always show bindings panel for simplicity
        float bx = 350.f;
        float by = m_startY;
        auto& im = InputManager::getInstance();
        for (std::size_t i = 0; i < m_actionsToShow.size(); ++i) {
            const auto act = m_actionsToShow[i];
            std::string name;
            switch (act) {
                case input::Action::MoveUp: name = "MoveUp"; break;
                case input::Action::MoveDown: name = "MoveDown"; break;
                case input::Action::MoveLeft: name = "MoveLeft"; break;
                case input::Action::MoveRight: name = "MoveRight"; break;
                case input::Action::Confirm: name = "Confirm"; break;
                case input::Action::Cancel: name = "Cancel"; break;
                default: name = "?"; break;
            }
            std::string binding = im.getBindingName(act);
            sf::Text bindTxt(font, name + ": " + binding, 18);
            bindTxt.setPosition(sf::Vector2f(bx, by + static_cast<float>(i) * (m_spacing - 6.f)));
            bindTxt.setFillColor(i == 0 ? sf::Color::Yellow : sf::Color::Cyan);
            window.draw(bindTxt);
        }

        if (m_waitingForRemap) {
            sf::Text instr(font, "Press a key or click a mouse button to rebind...", 18);
            instr.setFillColor(sf::Color::Yellow);
            instr.setPosition(sf::Vector2f(bx, by + static_cast<float>(m_actionsToShow.size()) * (m_spacing - 6.f)));
            window.draw(instr);
        }
    }
    try {
        // finished render
    } catch (const std::exception& e) {
        try { core::Logger::instance().error(std::string("OptionsMenu::render exception: ") + e.what()); } catch(...) {}
    } catch (...) {
        try { core::Logger::instance().error("OptionsMenu::render unknown exception"); } catch(...) {}
    }
}

} // namespace ui
