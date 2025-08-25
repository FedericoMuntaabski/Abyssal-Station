#ifndef ABYSSAL_STATION_SRC_CORE_CONFIGMANAGER_H
#define ABYSSAL_STATION_SRC_CORE_CONFIGMANAGER_H

#include <string>

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
    int musicVolume() const noexcept { return m_musicVolume; }
    int uiVolume() const noexcept { return m_uiVolume; }
    const std::string& language() const noexcept { return m_language; }
    int width() const noexcept { return m_width; }
    int height() const noexcept { return m_height; }
    bool fullscreen() const noexcept { return m_fullscreen; }
    int version() const noexcept { return m_version; }
    
    // Setters
    void setVolume(int vol) { m_volume = vol; }
    void setMusicVolume(int vol) { m_musicVolume = vol; }
    void setUiVolume(int vol) { m_uiVolume = vol; }
    void setLanguage(const std::string& lang) { m_language = lang; }
    void setResolution(int w, int h) { m_width = w; m_height = h; }
    void setFullscreen(bool fs) { m_fullscreen = fs; }

private:
    void setDefaults();

    std::string m_configDir;
    std::string m_configPath;

    // Data
    int m_volume{100};
    int m_musicVolume{80};
    int m_uiVolume{90};
    std::string m_language{"es"};
    int m_width{1920};
    int m_height{1080};
    bool m_fullscreen{false};
    int m_version{1};
    float m_notificationDuration{3.0f};
};

} // namespace core

#endif // ABYSSAL_STATION_SRC_CORE_CONFIGMANAGER_H
