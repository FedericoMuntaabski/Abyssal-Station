#ifndef ABYSSAL_STATION_SRC_CORE_DESIGNPATTERNSDEMO_H
#define ABYSSAL_STATION_SRC_CORE_DESIGNPATTERNSDEMO_H

#include "../entities/EntityFactory.h"
#include "../ai/BehaviorStrategy.h"
#include "../input/CommandSystem.h"
#include <memory>
#include <vector>

namespace core {

/**
 * Demonstration class showing the integration of design patterns.
 * This class serves as an example and documentation of how to use
 * the newly implemented Factory, Strategy, and Command patterns.
 */
class DesignPatternsDemo {
public:
    DesignPatternsDemo();
    ~DesignPatternsDemo() = default;
    
    // Factory Pattern Demo
    void demonstrateFactoryPattern();
    void createEntitiesFromConfig();
    void showFactoryConfiguration();
    
    // Strategy Pattern Demo
    void demonstrateStrategyPattern();
    void switchBehaviorStrategies();
    void compareBehaviorStrategies();
    
    // Command Pattern Demo
    void demonstrateCommandPattern();
    void recordAndPlaybackMacro();
    void showUndoRedoFunctionality();
    
    // Integration Demo
    void demonstratePatternIntegration();
    void createComplexScenario();
    
    // Performance and Analysis
    void analyzePatternPerformance();
    void generatePatternReport();

private:
    // Factory demonstration data
    std::vector<std::unique_ptr<entities::Entity>> m_demoEntities;
    
    // Strategy demonstration data
    std::vector<std::unique_ptr<ai::IBehaviorStrategy>> m_demoStrategies;
    
    // Command demonstration data
    std::unique_ptr<input::CommandSystem> m_commandSystem;
    
    // Helper methods
    void logDemo(const std::string& title, const std::string& description);
    void createSampleEntityConfigs();
    void setupBehaviorStrategies();
    void initializeCommandSystem();
};

} // namespace core

#endif // ABYSSAL_STATION_SRC_CORE_DESIGNPATTERNSDEMO_H
