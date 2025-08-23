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
        onEnter();
    }
}

void Menu::deactivate() {
    if (active_) {
        active_ = false;
        try {
            core::Logger::instance().info("Menu exited: " + name_);
        } catch (...) {}
        onExit();
    }
}

bool Menu::isActive() const noexcept { return active_; }

const std::string& Menu::name() const noexcept { return name_; }

} // namespace ui
