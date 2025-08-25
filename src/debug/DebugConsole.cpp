#include "DebugConsole.h"
#include "../entities/Player.h"
#include "../core/Logger.h"
#include <sstream>
#include <algorithm>
#include <ctime>

namespace debug {

DebugConsole::DebugConsole() {
    loadFont();
    initializeDefaultCommands();
    addInfo("Debug Console initialized. Type 'help' for available commands.");
}

void DebugConsole::loadFont() {
    m_fontLoaded = m_font.openFromFile("assets/fonts/Secundary_font.ttf");
    if (!m_fontLoaded) {
        core::Logger::instance().warning("DebugConsole: Failed to load font");
    }
}

void DebugConsole::toggle() {
    if (m_isOpen) {
        close();
    } else {
        open();
    }
}

void DebugConsole::open() {
    m_isOpen = true;
    m_inputBuffer.clear();
    core::Logger::instance().info("DebugConsole: Opened");
}

void DebugConsole::close() {
    m_isOpen = false;
    core::Logger::instance().info("DebugConsole: Closed");
}

void DebugConsole::handleEvent(sf::Event& event) {
    if (!m_isOpen) return;

    if (event.is<sf::Event::KeyPressed>()) {
        auto kp = event.getIf<sf::Event::KeyPressed>();
        
        switch (kp->code) {
            case sf::Keyboard::Key::F1:
                close();
                break;
                
            case sf::Keyboard::Key::Enter:
                if (!m_inputBuffer.empty()) {
                    addOutput("> " + m_inputBuffer, sf::Color::Yellow);
                    
                    // Add to command history
                    if (m_commandHistory.size() >= MAX_COMMAND_HISTORY) {
                        m_commandHistory.erase(m_commandHistory.begin());
                    }
                    m_commandHistory.push_back(m_inputBuffer);
                    m_historyIndex = -1;
                    
                    executeCommand(m_inputBuffer);
                    m_inputBuffer.clear();
                }
                break;
                
            case sf::Keyboard::Key::Up:
                navigateHistory(1);
                break;
                
            case sf::Keyboard::Key::Down:
                navigateHistory(-1);
                break;
                
            case sf::Keyboard::Key::Backspace:
                if (!m_inputBuffer.empty()) {
                    m_inputBuffer.pop_back();
                }
                break;
                
            default:
                break;
        }
    }
    
    if (event.is<sf::Event::TextEntered>()) {
        auto te = event.getIf<sf::Event::TextEntered>();
        char character = static_cast<char>(te->unicode);
        
        // Only accept printable characters, not control characters
        if (character >= 32 && character < 127 && character != '`') {
            m_inputBuffer += character;
        }
    }
}

void DebugConsole::update(float deltaTime) {
    // Remove old output lines if too many
    if (m_outputHistory.size() > MAX_OUTPUT_LINES) {
        m_outputHistory.erase(m_outputHistory.begin(), 
                             m_outputHistory.begin() + (m_outputHistory.size() - MAX_OUTPUT_LINES));
    }
}

void DebugConsole::render(sf::RenderWindow& window) {
    if (!m_isOpen || !m_fontLoaded) return;

    sf::Vector2f windowSize = static_cast<sf::Vector2f>(window.getSize());
    
    // Console background
    sf::RectangleShape background;
    background.setSize(sf::Vector2f(windowSize.x, m_consoleHeight));
    background.setPosition(sf::Vector2f(0.0f, 0.0f));
    background.setFillColor(sf::Color(0, 0, 0, 200));
    window.draw(background);
    
    // Border
    sf::RectangleShape border;
    border.setSize(sf::Vector2f(windowSize.x, 2.0f));
    border.setPosition(sf::Vector2f(0.0f, m_consoleHeight));
    border.setFillColor(sf::Color::Green);
    window.draw(border);
    
    float lineHeight = 16.0f;
    float padding = 10.0f;
    
    // Output history
    float yPos = m_consoleHeight - 40.0f; // Leave space for input line
    for (int i = static_cast<int>(m_outputHistory.size()) - 1; i >= 0 && yPos > padding; --i) {
        const auto& line = m_outputHistory[i];
        sf::Text text(m_font, line.text, 12);
        text.setFillColor(line.color);
        text.setPosition(sf::Vector2f(padding, yPos));
        window.draw(text);
        yPos -= lineHeight;
    }
    
    // Input line
    sf::Text inputPrompt(m_font, "> " + m_inputBuffer + "_", 14);
    inputPrompt.setFillColor(sf::Color::White);
    inputPrompt.setPosition(sf::Vector2f(padding, m_consoleHeight - 25.0f));
    window.draw(inputPrompt);
}

void DebugConsole::registerCommand(const std::string& name, CommandHandler handler, const std::string& description) {
    m_commands[name] = {handler, description};
    core::Logger::instance().info("DebugConsole: Registered command '" + name + "'");
}

void DebugConsole::executeCommand(const std::string& command) {
    std::string cmd;
    std::vector<std::string> args;
    parseCommand(command, cmd, args);
    
    auto it = m_commands.find(cmd);
    if (it != m_commands.end()) {
        try {
            it->second.handler(args);
        } catch (const std::exception& e) {
            addError("Command error: " + std::string(e.what()));
        }
    } else {
        addError("Unknown command: " + cmd + ". Type 'help' for available commands.");
    }
}

void DebugConsole::addOutput(const std::string& text, sf::Color color) {
    OutputLine line;
    line.text = text;
    line.color = color;
    line.timestamp = static_cast<float>(std::time(nullptr));
    m_outputHistory.push_back(line);
}

void DebugConsole::addError(const std::string& text) {
    addOutput("[ERROR] " + text, sf::Color::Red);
}

void DebugConsole::addInfo(const std::string& text) {
    addOutput("[INFO] " + text, sf::Color::Cyan);
}

void DebugConsole::addWarning(const std::string& text) {
    addOutput("[WARNING] " + text, sf::Color::Yellow);
}

void DebugConsole::parseCommand(const std::string& input, std::string& command, std::vector<std::string>& args) {
    std::istringstream iss(input);
    std::string token;
    
    if (iss >> command) {
        // Convert command to lowercase
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);
        
        // Parse arguments
        while (iss >> token) {
            args.push_back(token);
        }
    }
}

void DebugConsole::navigateHistory(int direction) {
    if (m_commandHistory.empty()) return;
    
    if (direction > 0) { // Up arrow - go back in history
        if (m_historyIndex == -1) {
            m_historyIndex = static_cast<int>(m_commandHistory.size()) - 1;
        } else if (m_historyIndex > 0) {
            m_historyIndex--;
        }
    } else { // Down arrow - go forward in history
        if (m_historyIndex != -1) {
            m_historyIndex++;
            if (m_historyIndex >= static_cast<int>(m_commandHistory.size())) {
                m_historyIndex = -1;
                m_inputBuffer.clear();
                return;
            }
        }
    }
    
    if (m_historyIndex >= 0 && m_historyIndex < static_cast<int>(m_commandHistory.size())) {
        m_inputBuffer = m_commandHistory[m_historyIndex];
    }
}

void DebugConsole::initializeDefaultCommands() {
    registerCommand("help", [this](const std::vector<std::string>& args) { cmdHelp(args); }, 
                   "Show available commands");
    
    registerCommand("clear", [this](const std::vector<std::string>& args) { cmdClear(args); }, 
                   "Clear console output");
    
    registerCommand("set_health", [this](const std::vector<std::string>& args) { cmdSetHealth(args); }, 
                   "Set player health (usage: set_health <value>)");
    
    registerCommand("teleport", [this](const std::vector<std::string>& args) { cmdTeleport(args); }, 
                   "Teleport player (usage: teleport <x> <y>)");
    
    registerCommand("god", [this](const std::vector<std::string>& args) { cmdGodMode(args); }, 
                   "Toggle god mode (usage: god <on|off>)");
    
    registerCommand("spawn_item", [this](const std::vector<std::string>& args) { cmdSpawnItem(args); }, 
                   "Spawn item at player location (usage: spawn_item <type>)");
    
    registerCommand("info", [this](const std::vector<std::string>& args) { cmdInfo(args); }, 
                   "Show player and game information");
}

void DebugConsole::cmdHelp(const std::vector<std::string>& args) {
    addInfo("Available commands:");
    for (const auto& pair : m_commands) {
        addOutput("  " + pair.first + " - " + pair.second.description, sf::Color::White);
    }
}

void DebugConsole::cmdClear(const std::vector<std::string>& args) {
    m_outputHistory.clear();
    addInfo("Console cleared.");
}

void DebugConsole::cmdSetHealth(const std::vector<std::string>& args) {
    if (args.empty()) {
        addError("Usage: set_health <value>");
        return;
    }
    
    if (!m_playerRef) {
        addError("Player reference not available");
        return;
    }
    
    try {
        int health = std::stoi(args[0]);
        // Note: This would need to be implemented in Player class
        addInfo("Player health set to " + std::to_string(health) + " (placeholder implementation)");
    } catch (const std::exception&) {
        addError("Invalid health value: " + args[0]);
    }
}

void DebugConsole::cmdTeleport(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        addError("Usage: teleport <x> <y>");
        return;
    }
    
    if (!m_playerRef) {
        addError("Player reference not available");
        return;
    }
    
    try {
        float x = std::stof(args[0]);
        float y = std::stof(args[1]);
        m_playerRef->setPosition(sf::Vector2f(x, y));
        addInfo("Player teleported to (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    } catch (const std::exception&) {
        addError("Invalid coordinates");
    }
}

void DebugConsole::cmdGodMode(const std::vector<std::string>& args) {
    if (args.empty()) {
        addError("Usage: god <on|off>");
        return;
    }
    
    std::string mode = args[0];
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
    
    if (mode == "on" || mode == "true" || mode == "1") {
        addInfo("God mode enabled (placeholder implementation)");
    } else if (mode == "off" || mode == "false" || mode == "0") {
        addInfo("God mode disabled (placeholder implementation)");
    } else {
        addError("Invalid god mode value. Use 'on' or 'off'");
    }
}

void DebugConsole::cmdSpawnItem(const std::vector<std::string>& args) {
    if (args.empty()) {
        addError("Usage: spawn_item <type>");
        addInfo("Available types: key, coin, health_potion, energy_crystal");
        return;
    }
    
    if (!m_playerRef) {
        addError("Player reference not available");
        return;
    }
    
    std::string itemType = args[0];
    sf::Vector2f playerPos = m_playerRef->position();
    addInfo("Spawned " + itemType + " at player location (placeholder implementation)");
}

void DebugConsole::cmdInfo(const std::vector<std::string>& args) {
    if (m_playerRef) {
        sf::Vector2f pos = m_playerRef->position();
        addInfo("Player Position: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")");
        addInfo("Player Size: (" + std::to_string(m_playerRef->size().x) + ", " + std::to_string(m_playerRef->size().y) + ")");
    } else {
        addWarning("Player reference not available");
    }
    
    addInfo("Debug Console v1.0");
    addInfo("Commands available: " + std::to_string(m_commands.size()));
}

} // namespace debug
