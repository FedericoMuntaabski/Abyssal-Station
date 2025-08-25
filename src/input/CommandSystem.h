#ifndef ABYSSAL_STATION_SRC_INPUT_COMMANDSYSTEM_H
#define ABYSSAL_STATION_SRC_INPUT_COMMANDSYSTEM_H

#include "Action.h"
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>
#include <stack>
#include <unordered_map>
#include <functional>
#include <string>
#include <chrono>

namespace core { class Game; }
namespace entities { class Player; class EntityManager; }
namespace scene { class SceneManager; }
namespace ui { class UIManager; }

namespace input {

/**
 * Command Pattern implementation for game actions.
 * Provides:
 * - Undo/Redo functionality
 * - Action queuing and batching
 * - Macro recording and playback
 * - Input remapping through commands
 * - Action validation and cancellation
 */

class ICommand {
public:
    virtual ~ICommand() = default;
    
    // Core command interface
    virtual bool execute() = 0;
    virtual bool undo() = 0;
    virtual bool canUndo() const = 0;
    
    // Command metadata
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual Action getAction() const = 0;
    
    // Validation and state
    virtual bool isValid() const = 0;
    virtual bool hasBeenExecuted() const = 0;
    virtual std::chrono::steady_clock::time_point getExecutionTime() const = 0;
    
    // Command merging for optimization
    virtual bool canMergeWith(const ICommand* other) const;
    virtual std::unique_ptr<ICommand> mergeWith(std::unique_ptr<ICommand> other);

protected:
    bool m_executed = false;
    std::chrono::steady_clock::time_point m_executionTime;
};

/**
 * Base class for game commands with common functionality
 */
class GameCommand : public ICommand {
public:
    explicit GameCommand(Action action, const std::string& name, const std::string& description = "");
    
    Action getAction() const override { return m_action; }
    std::string getName() const override { return m_name; }
    std::string getDescription() const override { return m_description; }
    bool hasBeenExecuted() const override { return m_executed; }
    std::chrono::steady_clock::time_point getExecutionTime() const override { return m_executionTime; }

protected:
    Action m_action;
    std::string m_name;
    std::string m_description;
};

/**
 * Movement commands for player
 */
class MoveCommand : public GameCommand {
public:
    MoveCommand(Action action, entities::Player* player, const sf::Vector2f& direction, float deltaTime);
    
    bool execute() override;
    bool undo() override;
    bool canUndo() const override { return true; }
    bool isValid() const override;
    
    bool canMergeWith(const ICommand* other) const override;
    std::unique_ptr<ICommand> mergeWith(std::unique_ptr<ICommand> other) override;

private:
    entities::Player* m_player;
    sf::Vector2f m_direction;
    sf::Vector2f m_previousPosition;
    sf::Vector2f m_newPosition;
    float m_deltaTime;
    static constexpr float MERGE_TIME_THRESHOLD = 0.1f; // seconds
};

/**
 * Interaction commands
 */
class InteractCommand : public GameCommand {
public:
    InteractCommand(entities::Player* player, entities::EntityManager* entityManager);
    
    bool execute() override;
    bool undo() override;
    bool canUndo() const override { return m_canUndo; }
    bool isValid() const override;

private:
    entities::Player* m_player;
    entities::EntityManager* m_entityManager;
    bool m_canUndo = false;
    std::string m_undoData; // JSON serialized undo information
};

/**
 * UI navigation commands
 */
class UICommand : public GameCommand {
public:
    enum class UIActionType {
        Navigate,
        Confirm,
        Cancel,
        ShowMenu,
        HideMenu
    };
    
    UICommand(Action action, ui::UIManager* uiManager, UIActionType uiAction, const std::string& parameter = "");
    
    bool execute() override;
    bool undo() override;
    bool canUndo() const override;
    bool isValid() const override;

private:
    ui::UIManager* m_uiManager;
    UIActionType m_uiActionType;
    std::string m_parameter;
    std::string m_previousState; // For undo
};

/**
 * Scene management commands
 */
class SceneCommand : public GameCommand {
public:
    enum class SceneActionType {
        Push,
        Pop,
        Replace,
        Pause,
        Resume
    };
    
    SceneCommand(Action action, scene::SceneManager* sceneManager, SceneActionType sceneAction, const std::string& sceneName = "");
    
    bool execute() override;
    bool undo() override;
    bool canUndo() const override;
    bool isValid() const override;

private:
    scene::SceneManager* m_sceneManager;
    SceneActionType m_sceneActionType;
    std::string m_sceneName;
    std::string m_previousSceneState; // For undo
};

/**
 * Macro command for recording and playing back sequences
 */
class MacroCommand : public GameCommand {
public:
    explicit MacroCommand(const std::string& name, const std::string& description = "");
    
    void addCommand(std::unique_ptr<ICommand> command);
    void clear();
    size_t getCommandCount() const { return m_commands.size(); }
    
    bool execute() override;
    bool undo() override;
    bool canUndo() const override;
    bool isValid() const override;

private:
    std::vector<std::unique_ptr<ICommand>> m_commands;
    size_t m_executedCommands = 0;
};

/**
 * Command system manager
 */
class CommandSystem {
public:
    explicit CommandSystem(size_t maxHistorySize = 1000);
    ~CommandSystem() = default;
    
    // Command execution
    bool executeCommand(std::unique_ptr<ICommand> command);
    bool executeImmediate(std::unique_ptr<ICommand> command); // Execute without adding to history
    
    // Undo/Redo functionality
    bool undo();
    bool redo();
    bool canUndo() const;
    bool canRedo() const;
    void clearHistory();
    
    // History management
    size_t getHistorySize() const { return m_commandHistory.size(); }
    size_t getMaxHistorySize() const { return m_maxHistorySize; }
    void setMaxHistorySize(size_t size);
    std::vector<std::string> getCommandHistory() const;
    
    // Command queuing and batching
    void queueCommand(std::unique_ptr<ICommand> command);
    void executeBatch();
    void clearQueue();
    size_t getQueueSize() const { return m_commandQueue.size(); }
    
    // Macro recording
    void startRecording(const std::string& macroName);
    void stopRecording();
    bool isRecording() const { return m_recording; }
    std::unique_ptr<MacroCommand> getMacro(const std::string& name);
    void saveMacro(const std::string& name, std::unique_ptr<MacroCommand> macro);
    std::vector<std::string> getMacroNames() const;
    
    // Command validation and cancellation
    void cancelCurrentCommands();
    void setValidationCallback(std::function<bool(const ICommand*)> callback);
    
    // Statistics and debugging
    struct Statistics {
        size_t totalExecuted = 0;
        size_t totalUndone = 0;
        size_t totalRedone = 0;
        size_t totalCancelled = 0;
        float averageExecutionTime = 0.0f;
    };
    Statistics getStatistics() const { return m_statistics; }
    void resetStatistics();

private:
    // History management
    std::vector<std::unique_ptr<ICommand>> m_commandHistory;
    size_t m_currentHistoryIndex = 0;
    size_t m_maxHistorySize;
    
    // Command queuing
    std::vector<std::unique_ptr<ICommand>> m_commandQueue;
    
    // Macro recording
    bool m_recording = false;
    std::string m_currentMacroName;
    std::unique_ptr<MacroCommand> m_currentMacro;
    std::unordered_map<std::string, std::unique_ptr<MacroCommand>> m_savedMacros;
    
    // Validation
    std::function<bool(const ICommand*)> m_validationCallback;
    
    // Statistics
    Statistics m_statistics;
    
    // Helper methods
    void addToHistory(std::unique_ptr<ICommand> command);
    void trimHistory();
    void updateStatistics(const ICommand* command, bool executed);
};

/**
 * Factory for creating commands from actions
 */
class CommandFactory {
public:
    // Set system references
    static void setPlayer(entities::Player* player);
    static void setEntityManager(entities::EntityManager* entityManager);
    static void setSceneManager(scene::SceneManager* sceneManager);
    static void setUIManager(ui::UIManager* uiManager);
    
    // Command creation
    static std::unique_ptr<ICommand> createCommand(Action action, float deltaTime = 0.0f);
    static std::unique_ptr<ICommand> createMoveCommand(Action action, float deltaTime);
    static std::unique_ptr<ICommand> createInteractCommand();
    static std::unique_ptr<ICommand> createUICommand(Action action);
    static std::unique_ptr<ICommand> createSceneCommand(Action action);
    
    // Custom command registration
    using CommandCreator = std::function<std::unique_ptr<ICommand>(Action, float)>;
    static void registerCommandCreator(Action action, CommandCreator creator);

private:
    static entities::Player* s_player;
    static entities::EntityManager* s_entityManager;
    static scene::SceneManager* s_sceneManager;
    static ui::UIManager* s_uiManager;
    static std::unordered_map<Action, CommandCreator> s_customCreators;
};

} // namespace input

#endif // ABYSSAL_STATION_SRC_INPUT_COMMANDSYSTEM_H
