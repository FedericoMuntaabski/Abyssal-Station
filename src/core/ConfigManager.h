#ifndef ABYSSAL_STATION_SRC_CORE_CONFIGMANAGER_H
#define ABYSSAL_STATION_SRC_CORE_CONFIGMANAGER_H

#include <string>
#include <algorithm>

namespace ui { class UIManager; }

namespace core {

class ConfigManager {
public:
    explicit ConfigManager(const std::string& configDir = "config");

    // Load config from disk. If file doesn't exist, defaults are used and file is created.
    bool loadConfig();

    // Save current config to disk.
    bool saveConfig() const;

    // Apply UI-related parts of the configuration to the provided UIManager.
    void applyConfig(ui::UIManager& ui);

    // Simple accessors
    int volume() const noexcept { return m_volume; }
    const std::string& language() const noexcept { return m_language; }
    int width() const noexcept { return m_width; }
    int height() const noexcept { return m_height; }
    int version() const noexcept { return m_version; }

    // Bindings persistence helpers (serialize/deserialize to InputManager)
    void loadBindingsToInput();
    void saveBindingsFromInput() const;

    // Mutators used by UI
    void setVolumeValue(int v) { m_volume = std::clamp(v, 0, 100); }

private:
    void setDefaults();

    std::string m_configDir;
    std::string m_configPath;

    // Data
    int m_volume{100};
    std::string m_language{"es"};
    int m_width{1920};
    int m_height{1080};
    int m_version{1};
    float m_notificationDuration{3.0f};
};

} // namespace core

#endif // ABYSSAL_STATION_SRC_CORE_CONFIGMANAGER_H
