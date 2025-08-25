#ifndef ABYSSAL_STATION_SRC_GAMEPLAY_ACHIEVEMENT_MANAGER_H
#define ABYSSAL_STATION_SRC_GAMEPLAY_ACHIEVEMENT_MANAGER_H

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

namespace gameplay {

enum class AchievementType {
    Collection,
    Combat,
    Exploration,
    Survival,
    Puzzle
};

struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    AchievementType type;
    int targetValue;
    int currentProgress;
    bool unlocked;
    
    Achievement(const std::string& id, const std::string& name, const std::string& description, 
                AchievementType type, int targetValue)
        : id(id), name(name), description(description), type(type), 
          targetValue(targetValue), currentProgress(0), unlocked(false) {}
};

class AchievementManager {
public:
    AchievementManager();
    ~AchievementManager() = default;

    // Achievement management
    void initializeAchievements();
    void updateProgress(const std::string& achievementId, int progress = 1);
    void unlockAchievement(const std::string& achievementId);
    
    // Query methods
    bool isUnlocked(const std::string& achievementId) const;
    int getProgress(const std::string& achievementId) const;
    std::vector<Achievement> getUnlockedAchievements() const;
    std::vector<Achievement> getAllAchievements() const;
    
    // Event callbacks
    void setOnAchievementUnlocked(std::function<void(const Achievement&)> callback);
    
    // Persistence
    void saveAchievements(const std::string& filename = "saves/achievements.json");
    void loadAchievements(const std::string& filename = "saves/achievements.json");

private:
    std::unordered_map<std::string, std::unique_ptr<Achievement>> m_achievements;
    std::function<void(const Achievement&)> m_onAchievementUnlocked;
    
    void checkAndUnlock(Achievement& achievement);
};

} // namespace gameplay

#endif // ABYSSAL_STATION_SRC_GAMEPLAY_ACHIEVEMENT_MANAGER_H
