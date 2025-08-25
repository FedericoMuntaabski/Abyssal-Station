#include "DebugOverlay.h"
#include "../entities/EntityManager.h"
#include "../ai/EnemyManager.h"
#include "../collisions/CollisionManager.h"
#include "../core/Logger.h"
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#endif

namespace debug {

DebugOverlay::DebugOverlay() : m_lastUpdate(std::chrono::steady_clock::now()) {
    loadFont();
}

void DebugOverlay::loadFont() {
    m_fontLoaded = m_font.openFromFile("assets/fonts/Secundary_font.ttf");
    if (!m_fontLoaded) {
        core::Logger::instance().warning("DebugOverlay: Failed to load font");
    }
}

void DebugOverlay::toggle() {
    m_isVisible = !m_isVisible;
    if (m_isVisible) {
        core::Logger::instance().info("DebugOverlay: Enabled");
    } else {
        core::Logger::instance().info("DebugOverlay: Disabled");
    }
}

void DebugOverlay::update(float deltaTime) {
    if (!m_isVisible) return;
    
    updatePerformanceMetrics(deltaTime);
    updateGameMetrics();
}

void DebugOverlay::render(sf::RenderWindow& window) {
    if (!m_isVisible || !m_fontLoaded) return;
    
    // Background panel
    float panelWidth = 300.0f;
    float panelHeight = static_cast<float>(m_metrics.size() + 2) * m_lineHeight + 20.0f;
    
    sf::RectangleShape background;
    background.setSize(sf::Vector2f(panelWidth, panelHeight));
    background.setPosition(m_position - sf::Vector2f(5.0f, 5.0f));
    background.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(background);
    
    // Border
    sf::RectangleShape border;
    border.setSize(sf::Vector2f(panelWidth, panelHeight));
    border.setPosition(m_position - sf::Vector2f(5.0f, 5.0f));
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(1.0f);
    border.setOutlineColor(sf::Color::Green);
    window.draw(border);
    
    // Title
    sf::Text title(m_font, "DEBUG OVERLAY", 14);
    title.setFillColor(sf::Color::Green);
    title.setStyle(sf::Text::Bold);
    title.setPosition(m_position);
    window.draw(title);
    
    // Metrics
    float yOffset = m_lineHeight + 5.0f;
    for (const auto& metric : m_metrics) {
        sf::Text text(m_font, metric.name + ": " + metric.value, 12);
        text.setFillColor(metric.color);
        text.setPosition(m_position + sf::Vector2f(0.0f, yOffset));
        window.draw(text);
        yOffset += m_lineHeight;
    }
}

void DebugOverlay::setCustomMetric(const std::string& name, const std::string& value) {
    // Update existing metric or add new one
    for (auto& metric : m_metrics) {
        if (metric.name == name) {
            metric.value = value;
            return;
        }
    }
    addMetric(name, value);
}

void DebugOverlay::updateFrameTime(float frameTime) {
    m_frameTime = frameTime;
}

void DebugOverlay::updatePerformanceMetrics(float deltaTime) {
    clearMetrics();
    
    // FPS calculation
    m_fpsUpdateTimer += deltaTime;
    if (m_fpsUpdateTimer >= FPS_UPDATE_INTERVAL) {
        m_fps = 1.0f / deltaTime;
        m_fpsUpdateTimer = 0.0f;
    }
    
    addMetric("FPS", formatFloat(m_fps, 1), 
              m_fps >= 60.0f ? sf::Color::Green : 
              m_fps >= 30.0f ? sf::Color::Yellow : sf::Color::Red);
    
    addMetric("Frame Time", formatFloat(deltaTime * 1000.0f, 2) + " ms", sf::Color::Cyan);
    
    // Memory usage
    size_t memoryUsage = getMemoryUsage();
    addMetric("Memory", formatMemorySize(memoryUsage), 
              memoryUsage < 256 * 1024 * 1024 ? sf::Color::Green : 
              memoryUsage < 512 * 1024 * 1024 ? sf::Color::Yellow : sf::Color::Red);
}

void DebugOverlay::updateGameMetrics() {
    // Entity metrics
    if (m_entityManager) {
        size_t entityCount = 0; // EntityManager would need a getEntityCount() method
        addMetric("Entities", std::to_string(entityCount), sf::Color::White);
    }
    
    // AI metrics
    if (m_enemyManager) {
        size_t enemyCount = m_enemyManager->enemies().size();
        addMetric("Enemies", std::to_string(enemyCount), sf::Color::White);
        
        // AI state information
        size_t activeEnemies = 0;
        for (const auto& enemy : m_enemyManager->enemies()) {
            if (enemy) {
                activeEnemies++;
            }
        }
        addMetric("Active AI", std::to_string(activeEnemies), sf::Color::White);
    }
    
    // Collision metrics
    if (m_collisionManager) {
        // These would need to be exposed by CollisionManager
        addMetric("Colliders", "N/A", sf::Color(128, 128, 128));
        addMetric("Collision Checks", "N/A", sf::Color(128, 128, 128));
    }
}

void DebugOverlay::addMetric(const std::string& name, const std::string& value, sf::Color color) {
    Metric metric;
    metric.name = name;
    metric.value = value;
    metric.color = color;
    m_metrics.push_back(metric);
}

void DebugOverlay::clearMetrics() {
    m_metrics.clear();
}

std::string DebugOverlay::formatMemorySize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 3) {
        size /= 1024.0;
        unitIndex++;
    }
    
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << size << " " << units[unitIndex];
    return ss.str();
}

std::string DebugOverlay::formatFloat(float value, int precision) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

size_t DebugOverlay::getMemoryUsage() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss * 1024; // Convert KB to bytes on Linux
    }
    return 0;
#endif
}

} // namespace debug
