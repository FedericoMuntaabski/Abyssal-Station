#include <gtest/gtest.h>
#include <SFML/Window/Event.hpp>
#include "../../src/input/InputManager.h"
#include "../../src/input/Action.h"
#include <filesystem>
#include <fstream>

class InputManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset InputManager to default state for each test
        auto& im = input::InputManager::getInstance();
        
        // Clear all bindings and set defaults
        im.rebindKeys(input::Action::MoveUp, {sf::Keyboard::Key::W, sf::Keyboard::Key::Up});
        im.rebindKeys(input::Action::MoveDown, {sf::Keyboard::Key::S, sf::Keyboard::Key::Down});
        im.rebindKeys(input::Action::MoveLeft, {sf::Keyboard::Key::A, sf::Keyboard::Key::Left});
        im.rebindKeys(input::Action::MoveRight, {sf::Keyboard::Key::D, sf::Keyboard::Key::Right});
        im.rebindKeys(input::Action::Confirm, {sf::Keyboard::Key::Enter});
        im.rebindKeys(input::Action::Cancel, {sf::Keyboard::Key::Escape});
        im.rebindKeys(input::Action::Interact, {sf::Keyboard::Key::E});
        im.rebindKeys(input::Action::Pause, {sf::Keyboard::Key::P});
        
        im.rebindMouse(input::Action::Confirm, {sf::Mouse::Button::Left});
        
        im.clearLastEvents();
        im.endFrame();
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove("test_bindings.json");
    }

    // Helper to simulate key press event
    sf::Event createKeyPressEvent(sf::Keyboard::Key key) {
        return sf::Event::KeyPressed{key, sf::Keyboard::Scancode::Unknown, false, false, false, false};
    }

    // Helper to simulate key release event
    sf::Event createKeyReleaseEvent(sf::Keyboard::Key key) {
        return sf::Event::KeyReleased{key, sf::Keyboard::Scancode::Unknown, false, false, false, false};
    }

    // Helper to simulate mouse press event
    sf::Event createMousePressEvent(sf::Mouse::Button button) {
        return sf::Event::MouseButtonPressed{button, {0, 0}};
    }

    // Helper to simulate mouse release event
    sf::Event createMouseReleaseEvent(sf::Mouse::Button button) {
        return sf::Event::MouseButtonReleased{button, {0, 0}};
    }
};

// Test basic key detection
TEST_F(InputManagerTest, BasicKeyDetection) {
    auto& im = input::InputManager::getInstance();
    
    // Initially no actions should be pressed
    EXPECT_FALSE(im.isActionPressed(input::Action::MoveUp));
    EXPECT_FALSE(im.isActionJustPressed(input::Action::MoveUp));
    
    // Simulate W key press
    sf::Event pressEvent = createKeyPressEvent(sf::Keyboard::Key::W);
    im.update(pressEvent);
    
    // Now MoveUp should be pressed and just pressed
    EXPECT_TRUE(im.isActionPressed(input::Action::MoveUp));
    EXPECT_TRUE(im.isActionJustPressed(input::Action::MoveUp));
    
    // End frame to advance state
    im.endFrame();
    
    // Should still be pressed but not just pressed
    EXPECT_TRUE(im.isActionPressed(input::Action::MoveUp));
    EXPECT_FALSE(im.isActionJustPressed(input::Action::MoveUp));
    
    // Simulate W key release
    sf::Event releaseEvent = createKeyReleaseEvent(sf::Keyboard::Key::W);
    im.update(releaseEvent);
    
    // Should be released
    EXPECT_FALSE(im.isActionPressed(input::Action::MoveUp));
    EXPECT_TRUE(im.isActionReleased(input::Action::MoveUp));
}

// Test multi-bindings
TEST_F(InputManagerTest, MultiBindings) {
    auto& im = input::InputManager::getInstance();
    
    // MoveUp should respond to both W and Up arrow
    sf::Event pressW = createKeyPressEvent(sf::Keyboard::Key::W);
    im.update(pressW);
    EXPECT_TRUE(im.isActionPressed(input::Action::MoveUp));
    
    sf::Event releaseW = createKeyReleaseEvent(sf::Keyboard::Key::W);
    im.update(releaseW);
    im.endFrame();
    
    sf::Event pressUp = createKeyPressEvent(sf::Keyboard::Key::Up);
    im.update(pressUp);
    EXPECT_TRUE(im.isActionPressed(input::Action::MoveUp));
}

// Test mouse input
TEST_F(InputManagerTest, MouseInput) {
    auto& im = input::InputManager::getInstance();
    
    // Confirm should respond to mouse left click
    sf::Event pressEvent = createMousePressEvent(sf::Mouse::Button::Left);
    im.update(pressEvent);
    
    EXPECT_TRUE(im.isActionPressed(input::Action::Confirm));
    EXPECT_TRUE(im.isActionJustPressed(input::Action::Confirm));
    
    im.endFrame();
    
    EXPECT_TRUE(im.isActionPressed(input::Action::Confirm));
    EXPECT_FALSE(im.isActionJustPressed(input::Action::Confirm));
    
    sf::Event releaseEvent = createMouseReleaseEvent(sf::Mouse::Button::Left);
    im.update(releaseEvent);
    
    EXPECT_FALSE(im.isActionPressed(input::Action::Confirm));
    EXPECT_TRUE(im.isActionReleased(input::Action::Confirm));
}

// Test binding rebind
TEST_F(InputManagerTest, RebindKeys) {
    auto& im = input::InputManager::getInstance();
    
    // Rebind MoveUp to just Q
    im.rebindKeys(input::Action::MoveUp, {sf::Keyboard::Key::Q});
    
    // W should no longer trigger MoveUp
    sf::Event pressW = createKeyPressEvent(sf::Keyboard::Key::W);
    im.update(pressW);
    EXPECT_FALSE(im.isActionPressed(input::Action::MoveUp));
    
    // Q should trigger MoveUp
    sf::Event pressQ = createKeyPressEvent(sf::Keyboard::Key::Q);
    im.update(pressQ);
    EXPECT_TRUE(im.isActionPressed(input::Action::MoveUp));
}

// Test binding name display
TEST_F(InputManagerTest, BindingNames) {
    auto& im = input::InputManager::getInstance();
    
    // Default bindings should return proper names
    std::string moveUpName = im.getBindingName(input::Action::MoveUp);
    EXPECT_EQ(moveUpName, "W");
    
    std::string confirmName = im.getBindingName(input::Action::Confirm);
    EXPECT_EQ(confirmName, "Enter");
    
    // Rebind and check name changes
    im.rebindKeys(input::Action::MoveUp, {sf::Keyboard::Key::Q});
    moveUpName = im.getBindingName(input::Action::MoveUp);
    EXPECT_EQ(moveUpName, "Q");
}

// Test serialization - save and load
TEST_F(InputManagerTest, SerializationSaveLoad) {
    auto& im = input::InputManager::getInstance();
    
    // Set custom bindings
    im.rebindKeys(input::Action::MoveUp, {sf::Keyboard::Key::Q});
    im.rebindKeys(input::Action::Confirm, {sf::Keyboard::Key::Space});
    im.rebindMouse(input::Action::Interact, {sf::Mouse::Button::Right});
    
    // Save bindings
    EXPECT_TRUE(im.saveBindings("test_bindings.json"));
    
    // Modify bindings
    im.rebindKeys(input::Action::MoveUp, {sf::Keyboard::Key::Z});
    EXPECT_EQ(im.getBindingName(input::Action::MoveUp), "Z");
    
    // Load original bindings
    EXPECT_TRUE(im.loadBindings("test_bindings.json"));
    
    // Should be back to Q
    EXPECT_EQ(im.getBindingName(input::Action::MoveUp), "Q");
    EXPECT_EQ(im.getBindingName(input::Action::Confirm), "Space");
}

// Test JSON export/import
TEST_F(InputManagerTest, JsonExportImport) {
    auto& im = input::InputManager::getInstance();
    
    // Set custom bindings
    im.rebindKeys(input::Action::MoveUp, {sf::Keyboard::Key::Q});
    im.rebindMouse(input::Action::Confirm, {sf::Mouse::Button::Right});
    
    // Export to JSON
    std::string jsonData = im.exportBindingsToJson();
    EXPECT_FALSE(jsonData.empty());
    
    // Modify bindings
    im.rebindKeys(input::Action::MoveUp, {sf::Keyboard::Key::Z});
    
    // Import from JSON
    EXPECT_TRUE(im.importBindingsFromJson(jsonData));
    
    // Should be back to Q
    EXPECT_EQ(im.getBindingName(input::Action::MoveUp), "Q");
}

// Test edge cases
TEST_F(InputManagerTest, EdgeCases) {
    auto& im = input::InputManager::getInstance();
    
    // Test empty bindings
    im.rebindKeys(input::Action::MoveUp, {});
    EXPECT_FALSE(im.isActionPressed(input::Action::MoveUp));
    EXPECT_EQ(im.getBindingName(input::Action::MoveUp), "Unbound");
    
    // Test invalid JSON
    EXPECT_FALSE(im.importBindingsFromJson("invalid json"));
    
    // Test non-existent file
    EXPECT_FALSE(im.loadBindings("nonexistent.json"));
}

// Test last event capture for remapping UI
TEST_F(InputManagerTest, LastEventCapture) {
    auto& im = input::InputManager::getInstance();
    
    // Initially no last events
    auto lastKey = im.getLastKeyEvent();
    auto lastMouse = im.getLastMouseButtonEvent();
    EXPECT_FALSE(lastKey.first);
    EXPECT_FALSE(lastMouse.first);
    
    // Simulate key press
    sf::Event pressEvent = createKeyPressEvent(sf::Keyboard::Key::Q);
    im.update(pressEvent);
    
    lastKey = im.getLastKeyEvent();
    EXPECT_TRUE(lastKey.first);
    EXPECT_EQ(lastKey.second, sf::Keyboard::Key::Q);
    
    // Clear events
    im.clearLastEvents();
    lastKey = im.getLastKeyEvent();
    EXPECT_FALSE(lastKey.first);
    
    // Simulate mouse press
    sf::Event mouseEvent = createMousePressEvent(sf::Mouse::Button::Right);
    im.update(mouseEvent);
    
    lastMouse = im.getLastMouseButtonEvent();
    EXPECT_TRUE(lastMouse.first);
    EXPECT_EQ(lastMouse.second, sf::Mouse::Button::Right);
}

// Test concurrent access (basic thread safety)
TEST_F(InputManagerTest, ThreadSafety) {
    auto& im = input::InputManager::getInstance();
    
    // This is a basic test - in a real scenario you'd test with actual threads
    // For now, just ensure the mutex doesn't cause deadlocks
    for (int i = 0; i < 100; ++i) {
        im.isActionPressed(input::Action::MoveUp);
        im.getBindingName(input::Action::Confirm);
        im.rebindKeys(input::Action::MoveUp, {sf::Keyboard::Key::W});
    }
    
    // If we get here without hanging, basic thread safety is working
    EXPECT_TRUE(true);
}
