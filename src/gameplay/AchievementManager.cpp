#include "AchievementManager.h"
#include "../core/Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace gameplay {

AchievementManager::AchievementManager() {
    initializeAchievements();
}

void AchievementManager::initializeAchievements() {
    // Collection achievements
    m_achievements["collector"] = std::make_unique<Achievement>(
        "collector", "Coleccionista", "Recoger 10 items", AchievementType::Collection, 10);
    
    m_achievements["item_hoarder"] = std::make_unique<Achievement>(
        "item_hoarder", "Acumulador", "Recoger 25 items", AchievementType::Collection, 25);
    
    m_achievements["treasure_hunter"] = std::make_unique<Achievement>(
        "treasure_hunter", "Cazador de Tesoros", "Recoger 50 items", AchievementType::Collection, 50);

    // Survival achievements
    m_achievements["survivor"] = std::make_unique<Achievement>(
        "survivor", "Superviviente", "Sobrevivir 5 minutos sin morir", AchievementType::Survival, 300); // 300 seconds
    
    m_achievements["veteran_survivor"] = std::make_unique<Achievement>(
        "veteran_survivor", "Superviviente Veterano", "Sobrevivir 10 minutos sin morir", AchievementType::Survival, 600);

    // Exploration achievements  
    m_achievements["explorer"] = std::make_unique<Achievement>(
        "explorer", "Explorador", "Visitar todas las areas de la estacion", AchievementType::Exploration, 5);
    
    m_achievements["station_master"] = std::make_unique<Achievement>(
        "station_master", "Maestro de la Estacion", "Descubrir todos los secretos", AchievementType::Exploration, 10);

    // Combat achievements
    m_achievements["first_blood"] = std::make_unique<Achievement>(
        "first_blood", "Primera Sangre", "Derrotar tu primer enemigo", AchievementType::Combat, 1);
    
    m_achievements["warrior"] = std::make_unique<Achievement>(
        "warrior", "Guerrero", "Derrotar 10 enemigos", AchievementType::Combat, 10);

    // Puzzle achievements
    m_achievements["puzzle_solver"] = std::make_unique<Achievement>(
        "puzzle_solver", "Solucionador", "Resolver tu primer puzzle", AchievementType::Puzzle, 1);
    
    m_achievements["master_puzzler"] = std::make_unique<Achievement>(
        "master_puzzler", "Maestro de Puzzles", "Resolver 5 puzzles", AchievementType::Puzzle, 5);

    core::Logger::instance().info("AchievementManager: Initialized " + std::to_string(m_achievements.size()) + " achievements");
}

void AchievementManager::updateProgress(const std::string& achievementId, int progress) {
    auto it = m_achievements.find(achievementId);
    if (it != m_achievements.end() && !it->second->unlocked) {
        it->second->currentProgress += progress;
        checkAndUnlock(*it->second);
        
        core::Logger::instance().info("AchievementManager: Updated progress for '" + achievementId + "' to " + 
                                     std::to_string(it->second->currentProgress) + "/" + std::to_string(it->second->targetValue));
    }
}

void AchievementManager::unlockAchievement(const std::string& achievementId) {
    auto it = m_achievements.find(achievementId);
    if (it != m_achievements.end() && !it->second->unlocked) {
        it->second->unlocked = true;
        it->second->currentProgress = it->second->targetValue;
        
        core::Logger::instance().info("AchievementManager: Achievement unlocked - " + it->second->name);
        
        if (m_onAchievementUnlocked) {
            m_onAchievementUnlocked(*it->second);
        }
    }
}

void AchievementManager::checkAndUnlock(Achievement& achievement) {
    if (!achievement.unlocked && achievement.currentProgress >= achievement.targetValue) {
        achievement.unlocked = true;
        
        core::Logger::instance().info("AchievementManager: Achievement auto-unlocked - " + achievement.name);
        
        if (m_onAchievementUnlocked) {
            m_onAchievementUnlocked(achievement);
        }
    }
}

bool AchievementManager::isUnlocked(const std::string& achievementId) const {
    auto it = m_achievements.find(achievementId);
    return (it != m_achievements.end()) ? it->second->unlocked : false;
}

int AchievementManager::getProgress(const std::string& achievementId) const {
    auto it = m_achievements.find(achievementId);
    return (it != m_achievements.end()) ? it->second->currentProgress : 0;
}

std::vector<Achievement> AchievementManager::getUnlockedAchievements() const {
    std::vector<Achievement> unlocked;
    for (const auto& pair : m_achievements) {
        if (pair.second->unlocked) {
            unlocked.push_back(*pair.second);
        }
    }
    return unlocked;
}

std::vector<Achievement> AchievementManager::getAllAchievements() const {
    std::vector<Achievement> all;
    for (const auto& pair : m_achievements) {
        all.push_back(*pair.second);
    }
    return all;
}

void AchievementManager::setOnAchievementUnlocked(std::function<void(const Achievement&)> callback) {
    m_onAchievementUnlocked = callback;
}

void AchievementManager::saveAchievements(const std::string& filename) {
    try {
        nlohmann::json achievementsJson;
        
        for (const auto& pair : m_achievements) {
            const auto& achievement = *pair.second;
            achievementsJson[achievement.id] = {
                {"progress", achievement.currentProgress},
                {"unlocked", achievement.unlocked}
            };
        }
        
        // Ensure saves directory exists
        std::filesystem::create_directories(std::filesystem::path(filename).parent_path());
        
        std::ofstream file(filename);
        if (file.is_open()) {
            file << achievementsJson.dump(2);
            core::Logger::instance().info("AchievementManager: Achievements saved to " + filename);
        } else {
            core::Logger::instance().warning("AchievementManager: Could not open file for writing: " + filename);
        }
    } catch (const std::exception& e) {
        core::Logger::instance().error("AchievementManager: Error saving achievements - " + std::string(e.what()));
    }
}

void AchievementManager::loadAchievements(const std::string& filename) {
    try {
        if (!std::filesystem::exists(filename)) {
            core::Logger::instance().info("AchievementManager: No existing achievement file found, using defaults");
            return;
        }
        
        std::ifstream file(filename);
        if (!file.is_open()) {
            core::Logger::instance().warning("AchievementManager: Could not open achievement file: " + filename);
            return;
        }
        
        nlohmann::json achievementsJson;
        file >> achievementsJson;
        
        for (const auto& [id, data] : achievementsJson.items()) {
            auto it = m_achievements.find(id);
            if (it != m_achievements.end()) {
                it->second->currentProgress = data.value("progress", 0);
                it->second->unlocked = data.value("unlocked", false);
            }
        }
        
        core::Logger::instance().info("AchievementManager: Achievements loaded from " + filename);
    } catch (const std::exception& e) {
        core::Logger::instance().error("AchievementManager: Error loading achievements - " + std::string(e.what()));
    }
}

} // namespace gameplay
