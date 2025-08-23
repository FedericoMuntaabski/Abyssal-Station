#include "Menu.h"
#include "../core/Logger.h"

namespace ui {

Menu::Menu(const std::string& name)
    : active_(false), name_(name) {}

Menu::~Menu() = default;

void Menu::activate() {
    if (!active_) {
        active_ = true;
        try {
            core::Logger::instance().info("Menu entered: " + name_);
        } catch (...) {}
        try {
            onEnter();
        } catch (const std::exception& e) {
            try { core::Logger::instance().error(std::string("Menu::onEnter exception: ") + e.what()); } catch(...) {}
        } catch (...) {
            try { core::Logger::instance().error("Menu::onEnter unknown exception"); } catch(...) {}
        }
    }
}

void Menu::deactivate() {
    if (active_) {
        active_ = false;
        try {
            core::Logger::instance().info("Menu exited: " + name_);
        } catch (...) {}
        try {
            onExit();
        } catch (const std::exception& e) {
            try { core::Logger::instance().error(std::string("Menu::onExit exception: ") + e.what()); } catch(...) {}
        } catch (...) {
            try { core::Logger::instance().error("Menu::onExit unknown exception"); } catch(...) {}
        }
    }
}

bool Menu::isActive() const noexcept { return active_; }

const std::string& Menu::name() const noexcept { return name_; }

} // namespace ui
