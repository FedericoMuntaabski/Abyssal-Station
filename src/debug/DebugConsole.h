#ifndef ABYSSAL_STATION_SRC_DEBUG_DEBUGCONSOLE_H
#define ABYSSAL_STATION_SRC_DEBUG_DEBUGCONSOLE_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace entities { class Player; }

namespace debug {

class DebugConsole {
public:
    using CommandHandler = std::function<void(const std::vector<std::string>&)>;
    
    DebugConsole();
    ~DebugConsole() = default;

    // Console state management
    void toggle();
    void open();
    void close();
    bool isOpen() const { return m_isOpen; }

    // Console operations
    void handleEvent(sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    // Command system
    void registerCommand(const std::string& name, CommandHandler handler, const std::string& description = "");
    void executeCommand(const std::string& command);

    // Output management
    void addOutput(const std::string& text, sf::Color color = sf::Color::White);
    void addError(const std::string& text);
    void addInfo(const std::string& text);
    void addWarning(const std::string& text);

    // Game entity references (for commands)
    void setPlayerReference(entities::Player* player) { m_playerRef = player; }

private:
    struct Command {
        CommandHandler handler;
        std::string description;
    };

    struct OutputLine {
        std::string text;
        sf::Color color;
        float timestamp;
    };

    // Console state
    bool m_isOpen{false};
    std::string m_inputBuffer;
    std::vector<OutputLine> m_outputHistory;
    std::vector<std::string> m_commandHistory;
    int m_historyIndex{-1};
    
    // Visual properties
    sf::Font m_font;
    bool m_fontLoaded{false};
    float m_consoleHeight{300.0f};
    static constexpr size_t MAX_OUTPUT_LINES = 50;
    static constexpr size_t MAX_COMMAND_HISTORY = 20;
    
    // Commands registry
    std::unordered_map<std::string, Command> m_commands;
    
    // Game references
    entities::Player* m_playerRef{nullptr};
    
    // Helper methods
    void loadFont();
    void initializeDefaultCommands();
    void parseCommand(const std::string& input, std::string& command, std::vector<std::string>& args);
    void scrollOutput(int direction);
    void navigateHistory(int direction);
    
    // Default command implementations
    void cmdHelp(const std::vector<std::string>& args);
    void cmdClear(const std::vector<std::string>& args);
    void cmdSetHealth(const std::vector<std::string>& args);
    void cmdTeleport(const std::vector<std::string>& args);
    void cmdGodMode(const std::vector<std::string>& args);
    void cmdSpawnItem(const std::vector<std::string>& args);
    void cmdInfo(const std::vector<std::string>& args);
};

} // namespace debug

#endif // ABYSSAL_STATION_SRC_DEBUG_DEBUGCONSOLE_H
