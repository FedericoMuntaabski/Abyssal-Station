#ifndef ABYSSAL_STATION_SRC_UI_OPTIONSMENU_H
#define ABYSSAL_STATION_SRC_UI_OPTIONSMENU_H

#include "Menu.h"
#include <vector>
#include <string>
#include "../input/Action.h"

namespace scene { class SceneManager; }

namespace ui {

class OptionsMenu : public Menu {
public:
    explicit OptionsMenu(scene::SceneManager* manager = nullptr);
    ~OptionsMenu() override = default;

    void handleInput() override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    void onEnter() override;
    void onExit() override;

    // simple getters/setters for tests or external UIManager
    int volume() const noexcept;
    void setVolume(int vol);

private:
    scene::SceneManager* m_manager{nullptr};
    std::vector<std::string> m_options;
    std::size_t m_selected{0};
    int m_volume{100}; // 0..100
    float m_startY{140.f};
    float m_spacing{36.f};
    void applyVolume();
    // Controls remapping
    bool m_waitingForRemap{false};
    input::Action m_actionToRemap{input::Action::MoveUp};
    std::vector<input::Action> m_actionsToShow{input::Action::MoveUp, input::Action::MoveDown, input::Action::MoveLeft, input::Action::MoveRight, input::Action::Confirm, input::Action::Cancel};
};

} // namespace ui

#endif // ABYSSAL_STATION_SRC_UI_OPTIONSMENU_H
