#include "ConfigManager.h"
#include "Logger.h"
#include <ui/UIManager.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include "../input/InputManager.h"
#include "../input/Action.h"
#include <algorithm>

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
    m_language = "es";
    m_width = 1920;
    m_height = 1080;
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

        if (j.contains("volume") && j["volume"].is_number_integer()) m_volume = j["volume"].get<int>();
        if (j.contains("language") && j["language"].is_string()) m_language = j["language"].get<std::string>();
        if (j.contains("resolution") && j["resolution"].is_object()) {
            auto &r = j["resolution"];
            if (r.contains("width") && r["width"].is_number_integer()) m_width = r["width"].get<int>();
            if (r.contains("height") && r["height"].is_number_integer()) m_height = r["height"].get<int>();
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
        j["volume"] = m_volume;
        j["language"] = m_language;
        j["resolution"] = { {"width", m_width}, {"height", m_height} };
        j["notificationDuration"] = m_notificationDuration;

        // Save bindings
        try {
            nlohmann::json binds = nlohmann::json::object();
            auto& im = input::InputManager::getInstance();
            for (int a = 0; a <= static_cast<int>(input::Action::Pause); ++a) {
                auto act = static_cast<input::Action>(a);
                auto keys = im.getKeyBindings(act);
                auto mice = im.getMouseBindings(act);
                nlohmann::json arr = nlohmann::json::array();
                for (auto k : keys) arr.push_back(static_cast<int>(k));
                for (auto m : mice) arr.push_back(1000 + static_cast<int>(m)); // mouse offset
                binds[std::to_string(a)] = arr;
            }
            j["bindings"] = binds;
        } catch (...) {
            // non-fatal
        }

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

void ConfigManager::saveBindingsFromInput() const
{
    // convenience wrapper that loads config, updates bindings section and writes file
    try {
        nlohmann::json j;
        std::ifstream in(m_configPath);
        if (in.good()) in >> j; else j = nlohmann::json::object();

        nlohmann::json binds = nlohmann::json::object();
        auto& im = input::InputManager::getInstance();
        for (int a = 0; a <= static_cast<int>(input::Action::Pause); ++a) {
            auto act = static_cast<input::Action>(a);
            auto keys = im.getKeyBindings(act);
            auto mice = im.getMouseBindings(act);
            nlohmann::json arr = nlohmann::json::array();
            for (auto k : keys) arr.push_back(static_cast<int>(k));
            for (auto m : mice) arr.push_back(1000 + static_cast<int>(m));
            binds[std::to_string(a)] = arr;
        }

        j["bindings"] = binds;

        std::ofstream out(m_configPath);
        if (out.good()) out << j.dump(4);
    } catch (...) {
        // ignore
    }
}

void ConfigManager::loadBindingsToInput()
{
    try {
        if (!std::filesystem::exists(m_configPath)) return;
        nlohmann::json j;
        std::ifstream in(m_configPath);
        if (!in.good()) return;
        in >> j;
        if (!j.contains("bindings")) return;
        auto& binds = j["bindings"];
        auto& im = input::InputManager::getInstance();
        for (auto it = binds.begin(); it != binds.end(); ++it) {
            int idx = std::stoi(it.key());
            auto act = static_cast<input::Action>(idx);
            std::vector<sf::Keyboard::Key> keys;
            std::vector<sf::Mouse::Button> mice;
            for (auto v : it.value()) {
                int vi = v.get<int>();
                if (vi >= 1000) {
                    mice.push_back(static_cast<sf::Mouse::Button>(vi - 1000));
                } else {
                    keys.push_back(static_cast<sf::Keyboard::Key>(vi));
                }
            }
            if (!keys.empty()) im.rebindKeys(act, keys);
            if (!mice.empty()) im.rebindMouse(act, mice);
        }
    } catch (const std::exception& ex) {
        core::Logger::instance().error(std::string("[config] Error loading bindings: ") + ex.what());
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
