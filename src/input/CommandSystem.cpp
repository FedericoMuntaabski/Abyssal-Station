#include "CommandSystem.h"
#include "../entities/Player.h"
#include "../entities/EntityManager.h"
#include "../scene/SceneManager.h"
#include "../ui/UIManager.h"
#include <algorithm>
#include <sstream>
#include <chrono>

namespace input {

// Static members for CommandFactory
entities::Player* CommandFactory::s_player = nullptr;
entities::EntityManager* CommandFactory::s_entityManager = nullptr;
scene::SceneManager* CommandFactory::s_sceneManager = nullptr;
ui::UIManager* CommandFactory::s_uiManager = nullptr;
std::unordered_map<Action, CommandFactory::CommandCreator> CommandFactory::s_customCreators;

//=====================================================================================
// ICommand base implementation
//=====================================================================================

bool ICommand::canMergeWith(const ICommand* other) const {
    return false; // Default: no merging
}

std::unique_ptr<ICommand> ICommand::mergeWith(std::unique_ptr<ICommand> other) {
    return std::move(other); // Default: return the other command
}

//=====================================================================================
// GameCommand implementation
//=====================================================================================

GameCommand::GameCommand(Action action, const std::string& name, const std::string& description)
    : m_action(action), m_name(name), m_description(description) {
}

//=====================================================================================
// MoveCommand implementation
//=====================================================================================

MoveCommand::MoveCommand(Action action, entities::Player* player, const sf::Vector2f& direction, float deltaTime)
    : GameCommand(action, "Move", "Player movement command"), 
      m_player(player), m_direction(direction), m_deltaTime(deltaTime) {
    if (m_player) {
        m_previousPosition = m_player->position();
    }
}

bool MoveCommand::execute() {
    if (!isValid()) {
        return false;
    }
    
    m_previousPosition = m_player->position();
    
    // Calculate movement
    sf::Vector2f movement = m_direction * m_player->speed() * m_deltaTime;
    m_newPosition = m_previousPosition + movement;
    
    // Apply movement
    m_player->setPosition(m_newPosition);
    
    m_executed = true;
    m_executionTime = std::chrono::steady_clock::now();
    return true;
}

bool MoveCommand::undo() {
    if (!m_executed || !m_player) {
        return false;
    }
    
    m_player->setPosition(m_previousPosition);
    m_executed = false;
    return true;
}

bool MoveCommand::isValid() const {
    return m_player != nullptr && (m_direction.x != 0.0f || m_direction.y != 0.0f);
}

bool MoveCommand::canMergeWith(const ICommand* other) const {
    const MoveCommand* otherMove = dynamic_cast<const MoveCommand*>(other);
    if (!otherMove || otherMove->m_player != m_player) {
        return false;
    }
    
    // Check if commands are close in time
    auto timeDiff = std::chrono::duration_cast<std::chrono::duration<float>>(
        otherMove->getExecutionTime() - getExecutionTime()).count();
    
    return std::abs(timeDiff) < MERGE_TIME_THRESHOLD;
}

std::unique_ptr<ICommand> MoveCommand::mergeWith(std::unique_ptr<ICommand> other) {
    auto otherMove = std::unique_ptr<MoveCommand>(dynamic_cast<MoveCommand*>(other.release()));
    if (!otherMove) {
        return std::move(other);
    }
    
    // Create a new merged command
    sf::Vector2f combinedDirection = m_direction + otherMove->m_direction;
    float combinedDelta = m_deltaTime + otherMove->m_deltaTime;
    
    auto merged = std::make_unique<MoveCommand>(m_action, m_player, combinedDirection, combinedDelta);
    merged->m_previousPosition = m_previousPosition; // Keep original starting position
    
    return std::move(merged);
}

//=====================================================================================
// InteractCommand implementation
//=====================================================================================

InteractCommand::InteractCommand(entities::Player* player, entities::EntityManager* entityManager)
    : GameCommand(Action::Interact, "Interact", "Player interaction command"),
      m_player(player), m_entityManager(entityManager) {
}

bool InteractCommand::execute() {
    if (!isValid()) {
        return false;
    }
    
    // Simple interaction logic - can be extended
    // For now, just record that interaction happened
    m_canUndo = true;
    m_undoData = "interaction_executed"; // Could serialize actual game state
    
    m_executed = true;
    m_executionTime = std::chrono::steady_clock::now();
    return true;
}

bool InteractCommand::undo() {
    if (!m_executed || !m_canUndo) {
        return false;
    }
    
    // Restore previous state (simplified)
    m_executed = false;
    return true;
}

bool InteractCommand::isValid() const {
    return m_player != nullptr && m_entityManager != nullptr;
}

//=====================================================================================
// UICommand implementation
//=====================================================================================

UICommand::UICommand(Action action, ui::UIManager* uiManager, UIActionType uiAction, const std::string& parameter)
    : GameCommand(action, "UI", "UI navigation command"),
      m_uiManager(uiManager), m_uiActionType(uiAction), m_parameter(parameter) {
}

bool UICommand::execute() {
    if (!isValid()) {
        return false;
    }
    
    // Store previous state for undo
    m_previousState = "previous_ui_state"; // Simplified - could store actual UI state
    
    // Execute UI action based on type
    switch (m_uiActionType) {
        case UIActionType::Navigate:
            // UI navigation logic
            break;
        case UIActionType::Confirm:
            // Confirmation logic
            break;
        case UIActionType::Cancel:
            // Cancel logic
            break;
        case UIActionType::ShowMenu:
            // Show menu logic
            break;
        case UIActionType::HideMenu:
            // Hide menu logic
            break;
    }
    
    m_executed = true;
    m_executionTime = std::chrono::steady_clock::now();
    return true;
}

bool UICommand::undo() {
    if (!m_executed) {
        return false;
    }
    
    // Restore previous UI state
    m_executed = false;
    return true;
}

bool UICommand::canUndo() const {
    // Most UI actions can be undone
    return m_uiActionType != UIActionType::Confirm; // Confirmations typically can't be undone
}

bool UICommand::isValid() const {
    return m_uiManager != nullptr;
}

//=====================================================================================
// SceneCommand implementation
//=====================================================================================

SceneCommand::SceneCommand(Action action, scene::SceneManager* sceneManager, SceneActionType sceneAction, const std::string& sceneName)
    : GameCommand(action, "Scene", "Scene management command"),
      m_sceneManager(sceneManager), m_sceneActionType(sceneAction), m_sceneName(sceneName) {
}

bool SceneCommand::execute() {
    if (!isValid()) {
        return false;
    }
    
    // Store previous state
    m_previousSceneState = "previous_scene_state"; // Simplified
    
    // Execute scene action
    switch (m_sceneActionType) {
        case SceneActionType::Push:
            // Push scene logic
            break;
        case SceneActionType::Pop:
            // Pop scene logic
            break;
        case SceneActionType::Replace:
            // Replace scene logic
            break;
        case SceneActionType::Pause:
            // Pause scene logic
            break;
        case SceneActionType::Resume:
            // Resume scene logic
            break;
    }
    
    m_executed = true;
    m_executionTime = std::chrono::steady_clock::now();
    return true;
}

bool SceneCommand::undo() {
    if (!m_executed) {
        return false;
    }
    
    // Restore previous scene state
    m_executed = false;
    return true;
}

bool SceneCommand::canUndo() const {
    // Most scene changes can be undone, except certain operations
    return m_sceneActionType != SceneActionType::Replace; // Scene replacement typically can't be undone safely
}

bool SceneCommand::isValid() const {
    return m_sceneManager != nullptr;
}

//=====================================================================================
// MacroCommand implementation
//=====================================================================================

MacroCommand::MacroCommand(const std::string& name, const std::string& description)
    : GameCommand(Action::Pause, name, description) {  // Use any valid action as placeholder
}

void MacroCommand::addCommand(std::unique_ptr<ICommand> command) {
    if (command) {
        m_commands.push_back(std::move(command));
    }
}

void MacroCommand::clear() {
    m_commands.clear();
    m_executedCommands = 0;
    m_executed = false;
}

bool MacroCommand::execute() {
    if (m_commands.empty()) {
        return false;
    }
    
    m_executedCommands = 0;
    for (auto& command : m_commands) {
        if (command && command->execute()) {
            m_executedCommands++;
        } else {
            // If any command fails, stop execution
            break;
        }
    }
    
    m_executed = m_executedCommands > 0;
    if (m_executed) {
        m_executionTime = std::chrono::steady_clock::now();
    }
    
    return m_executed;
}

bool MacroCommand::undo() {
    if (!m_executed || m_executedCommands == 0) {
        return false;
    }
    
    // Undo commands in reverse order
    bool allUndone = true;
    for (size_t i = m_executedCommands; i > 0; --i) {
        auto& command = m_commands[i - 1];
        if (command && command->canUndo()) {
            if (!command->undo()) {
                allUndone = false;
            }
        }
    }
    
    if (allUndone) {
        m_executed = false;
        m_executedCommands = 0;
    }
    
    return allUndone;
}

bool MacroCommand::canUndo() const {
    if (!m_executed || m_executedCommands == 0) {
        return false;
    }
    
    // Can undo if all executed commands can be undone
    for (size_t i = 0; i < m_executedCommands; ++i) {
        if (!m_commands[i] || !m_commands[i]->canUndo()) {
            return false;
        }
    }
    
    return true;
}

bool MacroCommand::isValid() const {
    return !m_commands.empty();
}

//=====================================================================================
// CommandSystem implementation
//=====================================================================================

CommandSystem::CommandSystem(size_t maxHistorySize) : m_maxHistorySize(maxHistorySize) {
    m_commandHistory.reserve(maxHistorySize);
}

bool CommandSystem::executeCommand(std::unique_ptr<ICommand> command) {
    if (!command) {
        return false;
    }
    
    // Validate command if callback is set
    if (m_validationCallback && !m_validationCallback(command.get())) {
        m_statistics.totalCancelled++;
        return false;
    }
    
    // Record command if recording macro
    if (m_recording && m_currentMacro) {
        auto commandCopy = CommandFactory::createCommand(command->getAction());
        if (commandCopy) {
            m_currentMacro->addCommand(std::move(commandCopy));
        }
    }
    
    // Execute command
    bool success = command->execute();
    updateStatistics(command.get(), success);
    
    if (success) {
        addToHistory(std::move(command));
    }
    
    return success;
}

bool CommandSystem::executeImmediate(std::unique_ptr<ICommand> command) {
    if (!command) {
        return false;
    }
    
    bool success = command->execute();
    updateStatistics(command.get(), success);
    
    return success;
}

bool CommandSystem::undo() {
    if (!canUndo()) {
        return false;
    }
    
    auto& command = m_commandHistory[m_currentHistoryIndex - 1];
    if (command && command->canUndo() && command->undo()) {
        m_currentHistoryIndex--;
        m_statistics.totalUndone++;
        return true;
    }
    
    return false;
}

bool CommandSystem::redo() {
    if (!canRedo()) {
        return false;
    }
    
    auto& command = m_commandHistory[m_currentHistoryIndex];
    if (command && command->execute()) {
        m_currentHistoryIndex++;
        m_statistics.totalRedone++;
        return true;
    }
    
    return false;
}

bool CommandSystem::canUndo() const {
    return m_currentHistoryIndex > 0 && 
           m_currentHistoryIndex <= m_commandHistory.size() &&
           m_commandHistory[m_currentHistoryIndex - 1] &&
           m_commandHistory[m_currentHistoryIndex - 1]->canUndo();
}

bool CommandSystem::canRedo() const {
    return m_currentHistoryIndex < m_commandHistory.size() &&
           m_commandHistory[m_currentHistoryIndex] &&
           m_commandHistory[m_currentHistoryIndex]->isValid();
}

void CommandSystem::clearHistory() {
    m_commandHistory.clear();
    m_currentHistoryIndex = 0;
}

void CommandSystem::setMaxHistorySize(size_t size) {
    m_maxHistorySize = size;
    trimHistory();
}

std::vector<std::string> CommandSystem::getCommandHistory() const {
    std::vector<std::string> history;
    history.reserve(m_commandHistory.size());
    
    for (const auto& command : m_commandHistory) {
        if (command) {
            history.push_back(command->getName() + ": " + command->getDescription());
        }
    }
    
    return history;
}

void CommandSystem::queueCommand(std::unique_ptr<ICommand> command) {
    if (command) {
        m_commandQueue.push_back(std::move(command));
    }
}

void CommandSystem::executeBatch() {
    for (auto& command : m_commandQueue) {
        executeCommand(std::move(command));
    }
    m_commandQueue.clear();
}

void CommandSystem::clearQueue() {
    m_commandQueue.clear();
}

void CommandSystem::startRecording(const std::string& macroName) {
    m_recording = true;
    m_currentMacroName = macroName;
    m_currentMacro = std::make_unique<MacroCommand>(macroName, "Recorded macro: " + macroName);
}

void CommandSystem::stopRecording() {
    if (m_recording && m_currentMacro) {
        saveMacro(m_currentMacroName, std::move(m_currentMacro));
    }
    m_recording = false;
    m_currentMacroName.clear();
    m_currentMacro.reset();
}

std::unique_ptr<MacroCommand> CommandSystem::getMacro(const std::string& name) {
    auto it = m_savedMacros.find(name);
    if (it != m_savedMacros.end()) {
        // Return a copy of the macro
        auto copy = std::make_unique<MacroCommand>(it->second->getName(), it->second->getDescription());
        // Note: This is simplified - in a real implementation, you'd clone all commands
        return copy;
    }
    return nullptr;
}

void CommandSystem::saveMacro(const std::string& name, std::unique_ptr<MacroCommand> macro) {
    m_savedMacros[name] = std::move(macro);
}

std::vector<std::string> CommandSystem::getMacroNames() const {
    std::vector<std::string> names;
    names.reserve(m_savedMacros.size());
    
    for (const auto& pair : m_savedMacros) {
        names.push_back(pair.first);
    }
    
    return names;
}

void CommandSystem::cancelCurrentCommands() {
    clearQueue();
    if (m_recording) {
        stopRecording();
    }
}

void CommandSystem::setValidationCallback(std::function<bool(const ICommand*)> callback) {
    m_validationCallback = callback;
}

void CommandSystem::resetStatistics() {
    m_statistics = Statistics{};
}

void CommandSystem::addToHistory(std::unique_ptr<ICommand> command) {
    // Clear any redo commands if we're adding a new command
    if (m_currentHistoryIndex < m_commandHistory.size()) {
        m_commandHistory.erase(m_commandHistory.begin() + m_currentHistoryIndex, m_commandHistory.end());
    }
    
    // Add new command
    m_commandHistory.push_back(std::move(command));
    m_currentHistoryIndex = m_commandHistory.size();
    
    // Trim history if needed
    trimHistory();
}

void CommandSystem::trimHistory() {
    if (m_commandHistory.size() > m_maxHistorySize) {
        size_t toRemove = m_commandHistory.size() - m_maxHistorySize;
        m_commandHistory.erase(m_commandHistory.begin(), m_commandHistory.begin() + toRemove);
        m_currentHistoryIndex = std::min(m_currentHistoryIndex, m_commandHistory.size());
    }
}

void CommandSystem::updateStatistics(const ICommand* command, bool executed) {
    if (executed) {
        m_statistics.totalExecuted++;
    } else {
        m_statistics.totalCancelled++;
    }
    
    // Update average execution time (simplified)
    if (command && executed) {
        // This would need actual timing measurement in a real implementation
        m_statistics.averageExecutionTime = 0.001f; // Placeholder
    }
}

//=====================================================================================
// CommandFactory implementation
//=====================================================================================

void CommandFactory::setPlayer(entities::Player* player) {
    s_player = player;
}

void CommandFactory::setEntityManager(entities::EntityManager* entityManager) {
    s_entityManager = entityManager;
}

void CommandFactory::setSceneManager(scene::SceneManager* sceneManager) {
    s_sceneManager = sceneManager;
}

void CommandFactory::setUIManager(ui::UIManager* uiManager) {
    s_uiManager = uiManager;
}

std::unique_ptr<ICommand> CommandFactory::createCommand(Action action, float deltaTime) {
    // Check for custom creators first
    auto it = s_customCreators.find(action);
    if (it != s_customCreators.end()) {
        return it->second(action, deltaTime);
    }
    
    // Create standard commands based on action type
    switch (action) {
        case Action::MoveUp:
        case Action::MoveDown:
        case Action::MoveLeft:
        case Action::MoveRight:
            return createMoveCommand(action, deltaTime);
            
        case Action::Interact:
            return createInteractCommand();
            
        case Action::Pause:   // Using Pause instead of Menu
        case Action::Cancel: 
            return createUICommand(action);
            
        default:
            return nullptr;
    }
}

std::unique_ptr<ICommand> CommandFactory::createMoveCommand(Action action, float deltaTime) {
    if (!s_player) {
        return nullptr;
    }
    
    sf::Vector2f direction(0.0f, 0.0f);
    switch (action) {
        case Action::MoveUp:    direction = sf::Vector2f(0.0f, -1.0f); break;
        case Action::MoveDown:  direction = sf::Vector2f(0.0f, 1.0f);  break;
        case Action::MoveLeft:  direction = sf::Vector2f(-1.0f, 0.0f); break;
        case Action::MoveRight: direction = sf::Vector2f(1.0f, 0.0f);  break;
        default: return nullptr;
    }
    
    return std::make_unique<MoveCommand>(action, s_player, direction, deltaTime);
}

std::unique_ptr<ICommand> CommandFactory::createInteractCommand() {
    if (!s_player || !s_entityManager) {
        return nullptr;
    }
    
    return std::make_unique<InteractCommand>(s_player, s_entityManager);
}

std::unique_ptr<ICommand> CommandFactory::createUICommand(Action action) {
    if (!s_uiManager) {
        return nullptr;
    }
    
    UICommand::UIActionType uiAction;
    switch (action) {
        case Action::Pause:   uiAction = UICommand::UIActionType::ShowMenu; break;  // Using Pause instead of Menu
        case Action::Cancel: uiAction = UICommand::UIActionType::Cancel; break;
        default: return nullptr;
    }
    
    return std::make_unique<UICommand>(action, s_uiManager, uiAction);
}

std::unique_ptr<ICommand> CommandFactory::createSceneCommand(Action action) {
    if (!s_sceneManager) {
        return nullptr;
    }
    
    // This would need to map actions to scene operations
    // For now, return nullptr as it's context-dependent
    return nullptr;
}

void CommandFactory::registerCommandCreator(Action action, CommandCreator creator) {
    s_customCreators[action] = creator;
}

} // namespace input
