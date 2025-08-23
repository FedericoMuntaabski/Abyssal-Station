#ifndef ABYSSAL_STATION_SRC_CORE_SAVEMANAGER_H
#define ABYSSAL_STATION_SRC_CORE_SAVEMANAGER_H

#include "GameState.h"
#include <string>

namespace core {

class SaveManager {
public:
    explicit SaveManager(const std::string& savesDir = "saves");

    // Save the provided state to a file (filename without path)
    bool saveGame(const GameState& state, const std::string& filename);

    // Load state from file into provided GameState. Returns false if file not found or parse error.
    bool loadGame(GameState& outState, const std::string& filename);

private:
    std::string m_savesDir;
    std::string makePath(const std::string& filename) const;
};

} // namespace core

#endif // ABYSSAL_STATION_SRC_CORE_SAVEMANAGER_H
