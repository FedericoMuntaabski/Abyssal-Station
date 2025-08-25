#include "ConfigManager.h"
#include "Logger.h"
#include <ui/UIManager.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace core {

ConfigManager::ConfigManager(const std::string& configDir)
    : m_configDir(configDir)
{
    m_configPath = m_configDir + "/config.json";
    setDefaults();
}

void ConfigManager::setDefaults()
{
    m_volume = 100;
    m_musicVolume = 80;
    m_uiVolume = 90;
    m_language = "es";
    m_width = 1920;
    m_height = 1080;
    m_fullscreen = false;
    m_version = 1;
    m_notificationDuration = 3.0f;
}

bool ConfigManager::loadConfig()
{
    try {
        if (!fs::exists(m_configDir)) {
            fs::create_directories(m_configDir);
            core::Logger::instance().info("[config] Created config directory: " + m_configDir);
        }

        if (!fs::exists(m_configPath)) {
            // Config missing -> use defaults and persist them
            setDefaults();
            core::Logger::instance().warning("[config] Usando valores por defecto; no se encontró: " + m_configPath);
            // Try save defaults but ignore failure (we still continue with safe defaults)
            saveConfig();
            return true;
        }

        std::ifstream in(m_configPath);
        if (!in.good()) {
            core::Logger::instance().error("[config] Failed to open config file for reading: " + m_configPath);
            return false;
        }

        json j;
        in >> j;

        // Versioning
        if (j.contains("version") && j["version"].is_number_integer()) {
            m_version = j["version"].get<int>();
        }

        if (j.contains("volume")) {
            if (j["volume"].is_number_integer()) {
                // Old format compatibility
                m_volume = j["volume"].get<int>();
            } else if (j["volume"].is_object()) {
                // New format with separate volumes
                auto &vol = j["volume"];
                if (vol.contains("master") && vol["master"].is_number_integer()) m_volume = vol["master"].get<int>();
                if (vol.contains("music") && vol["music"].is_number_integer()) m_musicVolume = vol["music"].get<int>();
                if (vol.contains("ui") && vol["ui"].is_number_integer()) m_uiVolume = vol["ui"].get<int>();
            }
        }
        if (j.contains("language") && j["language"].is_string()) m_language = j["language"].get<std::string>();
        if (j.contains("resolution") && j["resolution"].is_object()) {
            auto &r = j["resolution"];
            if (r.contains("width") && r["width"].is_number_integer()) m_width = r["width"].get<int>();
            if (r.contains("height") && r["height"].is_number_integer()) m_height = r["height"].get<int>();
        }
        if (j.contains("display") && j["display"].is_object()) {
            auto &d = j["display"];
            if (d.contains("fullscreen") && d["fullscreen"].is_boolean()) m_fullscreen = d["fullscreen"].get<bool>();
        }
        if (j.contains("notificationDuration") && j["notificationDuration"].is_number()) m_notificationDuration = j["notificationDuration"].get<float>();

        core::Logger::instance().info("[config] Loaded config from " + m_configPath);
        return true;
    } catch (const std::exception& ex) {
        core::Logger::instance().error(std::string("[config] Error al cargar config: ") + ex.what());
        // Safe fallback to defaults
        setDefaults();
        core::Logger::instance().warning("[config] Usando valores por defecto debido a error al leer el archivo");
        return false;
    }
}

bool ConfigManager::saveConfig() const
{
    try {
        if (!fs::exists(m_configDir)) {
            fs::create_directories(m_configDir);
            core::Logger::instance().info("[config] Created config directory: " + m_configDir);
        }

        json j;
        j["version"] = m_version;
        j["volume"] = { 
            {"master", m_volume}, 
            {"music", m_musicVolume}, 
            {"ui", m_uiVolume} 
        };
        j["language"] = m_language;
        j["resolution"] = { {"width", m_width}, {"height", m_height} };
        j["display"] = { {"fullscreen", m_fullscreen}, {"vsync", true} };
        j["notificationDuration"] = m_notificationDuration;

        std::ofstream out(m_configPath);
        if (!out.good()) {
            core::Logger::instance().error("[config] Failed to open config file for writing: " + m_configPath);
            return false;
        }

        out << j.dump(4);
        core::Logger::instance().info("[config] Saved config to " + m_configPath);
        return true;
    } catch (const std::exception& ex) {
        core::Logger::instance().error(std::string("[config] Exception while saving config: ") + ex.what());
        return false;
    }
}

void ConfigManager::applyConfig(ui::UIManager& ui)
{
    // Apply UI related fields
    ui.setNotificationDuration(m_notificationDuration);
    // Potentially other UI updates: paused state, language, etc.
    core::Logger::instance().info("[config] Applied config: volume=" + std::to_string(m_volume) + ", lang=" + m_language);
}

} // namespace core
