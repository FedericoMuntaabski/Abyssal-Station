#include "SaveManager.h"
#include "Logger.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace core {

SaveManager::SaveManager(const std::string& savesDir)
    : m_savesDir(savesDir)
{
}

std::string SaveManager::makePath(const std::string& filename) const
{
    return m_savesDir + "/" + filename;
}

bool SaveManager::saveGame(const GameState& state, const std::string& filename)
{
    try {
        if (!fs::exists(m_savesDir)) fs::create_directories(m_savesDir);

        std::string path = makePath(filename);
        json j = state;

        std::ofstream out(path);
        if (!out.good()) {
            core::Logger::instance().error("[save] Failed to open save file for writing: " + path);
            return false;
        }

        out << j.dump(4);
        core::Logger::instance().info("[save] Saved game to " + path);
        return true;
    } catch (const std::exception& ex) {
        core::Logger::instance().error(std::string("[save] Exception while saving: ") + ex.what());
        return false;
    }
}

bool SaveManager::loadGame(GameState& outState, const std::string& filename)
{
    try {
        std::string path = makePath(filename);
        if (!fs::exists(path)) {
            core::Logger::instance().warning("[save] Archivo no encontrado: " + path);
            return false;
        }

        std::ifstream in(path);
        if (!in.good()) {
            core::Logger::instance().error("[load] Failed to open save file for reading: " + path);
            return false;
        }

        json j;
        in >> j;
        outState = j.get<GameState>();
        core::Logger::instance().info("[load] Loaded game from " + path);
        return true;
    } catch (const std::exception& ex) {
        core::Logger::instance().error(std::string("[load] Error al cargar save: ") + ex.what());
        return false;
    }
}

} // namespace core
