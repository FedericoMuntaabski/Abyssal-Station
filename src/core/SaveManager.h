#ifndef ABYSSAL_STATION_SRC_CORE_SAVEMANAGER_H
#define ABYSSAL_STATION_SRC_CORE_SAVEMANAGER_H

#include "GameState.h"
#include <string>
#include <chrono>

namespace core {

class SaveManager {
public:
    explicit SaveManager(const std::string& savesDir = "saves");

    // Save the provided state to a file (filename without path)
    bool saveGame(const GameState& state, const std::string& filename);

    // Load state from file into provided GameState. Returns false if file not found or parse error.
    bool loadGame(GameState& outState, const std::string& filename);
    
    // Auto-save functionality
    void enableAutoSave(bool enabled) { m_autoSaveEnabled = enabled; }
    bool isAutoSaveEnabled() const { return m_autoSaveEnabled; }
    void setAutoSaveInterval(float seconds) { m_autoSaveIntervalSeconds = seconds; }
    float getAutoSaveInterval() const { return m_autoSaveIntervalSeconds; }
    
    // Update auto-save timer and trigger save if needed
    void update(float deltaTime, const GameState& currentState);
    
    // Check if enough time has passed for auto-save
    bool shouldAutoSave() const;
    
    // Perform auto-save (creates auto_save.json)
    bool performAutoSave(const GameState& state);

private:
    std::string m_savesDir;
    std::string makePath(const std::string& filename) const;
    
    // Auto-save settings
    bool m_autoSaveEnabled{true};
    float m_autoSaveIntervalSeconds{120.0f}; // 2 minutes default
    float m_timeSinceLastAutoSave{0.0f};
};

} // namespace core

#endif // ABYSSAL_STATION_SRC_CORE_SAVEMANAGER_H
