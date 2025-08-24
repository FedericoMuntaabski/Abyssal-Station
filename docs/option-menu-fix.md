# Options Menu and UI Fixes - Testing Guide

## Overview
This document describes the changes made to fix the options menu, pause menu, and various UI issues in Abyssal Station.

## Changes Made

### 1. Main Menu Title Enhancement
- **Change**: Added orange outline to the title "ABYSSAL-STATION"
- **Location**: `src/ui/MainMenu.cpp` - `renderTitle()` method
- **Details**: 
  - Title moved slightly to the right from upper-left corner (position: 80px from left)
  - Orange outline with 180 alpha (slightly opaque)
  - UTF-8 string support implemented

### 2. Hover Sound Implementation
- **Change**: Added hover sounds to all menu interactions
- **Affected Files**: 
  - `src/ui/MainMenu.cpp` - Enhanced existing hover sound system
  - `src/ui/PauseMenu.cpp` - Added complete hover sound system
  - `src/ui/OptionsMenu.cpp` - Added hover sound system
- **Details**:
  - Uses `hover_select.wav` from `assets/sounds/`
  - Plays on navigation (arrow keys) and mouse hover
  - Prevents sound overlap with status checks

### 3. UTF-8 Character Support
- **Change**: Fixed encoding issues for accented characters
- **Implementation**: 
  - All text rendering now uses `sf::String::fromUtf8(text.begin(), text.end())`
  - Proper support for Spanish characters like "ó", "ñ", etc.
- **Affected Files**: All menu rendering code

### 4. Options Menu Complete Rewrite
- **Change**: Simplified and stabilized the options menu
- **New Features**:
  - **Volumen Música**: Music volume control (0-100%)
  - **Volumen UI**: UI sounds volume control (0-100%)
  - **Resolución**: Resolution selection (1024x576, 1280x720, 1366x768, 1920x1080)
  - **Pantalla Completa**: Fullscreen toggle (Activada/Desactivada)
  - **Guardar**: Manual save settings
  - **Regresar**: Return to main menu (auto-saves)
- **Navigation**: Left/Right arrows adjust values, Up/Down navigate options
- **Background**: Reuses Main Menu background image

### 5. Pause Menu Improvements
- **Change**: Enhanced navigation and renamed exit option
- **Details**:
  - Renamed "Quit" to "Volver al menú principal"
  - Added hover sounds and visual animations
  - Improved mouse hover detection
  - Enhanced navigation with same logic as MainMenu
  - UTF-8 support for all text
  - "Volver al menú principal" returns to main menu instead of quitting

### 6. Configuration System Enhancement
- **Change**: Updated ConfigManager and config.json structure
- **New Structure**:
  ```json
  {
    "volume": {
      "master": 100,
      "music": 80,
      "ui": 90
    },
    "display": {
      "fullscreen": false,
      "vsync": true
    }
  }
  ```
- **New Methods**: Added setters for all configuration options

### 7. UIManager Event System Extension
- **Change**: Added `triggerReturnToMainMenu()` event
- **Purpose**: Allows pause menu to properly return to main menu

## Testing Instructions

### Prerequisites
1. Compile the project using the existing build system
2. Ensure `assets/sounds/hover_select.wav` and `assets/sounds/confirm.wav` exist
3. Ensure `assets/fonts/Main_font.ttf` and `assets/fonts/Secundary_font.ttf` exist

### Test Cases

#### 1. Main Menu Title Test
**Expected Result**: 
- Title shows orange outline around white text
- Title positioned ~80px from left edge (not touching border)
- No encoding issues with any characters

**Steps**:
1. Launch game
2. Observe main menu title
3. Verify outline is visible and orange-colored

#### 2. Hover Sound Test
**Expected Result**: 
- Hover sound plays when moving cursor over menu items
- Hover sound plays when using arrow keys to navigate
- Sound doesn't overlap/spam

**Steps**:
1. Main Menu: Move mouse over buttons → should hear hover sound
2. Main Menu: Use arrow keys → should hear hover sound on navigation
3. Open Options Menu: Repeat tests
4. Open Pause Menu (Escape in game): Repeat tests

#### 3. Options Menu Functionality Test
**Expected Result**:
- Menu opens without crashing
- All options are interactive with Left/Right arrows
- Values persist after save and restart
- Background is same as Main Menu

**Steps**:
1. Main Menu → "Opciones" → should open without crash
2. Navigate to "Volumen Música" → use Left/Right arrows → value should change
3. Navigate to "Volumen UI" → use Left/Right arrows → value should change
4. Navigate to "Resolución" → use Left/Right arrows → resolution should cycle
5. Navigate to "Pantalla Completa" → use Left/Right arrows → should toggle
6. Select "Guardar" → should save settings
7. Select "Regresar" → should return to main menu
8. Restart game → verify settings persisted

#### 4. Pause Menu Test
**Expected Result**:
- Navigation works smoothly with hover sounds
- "Volver al menú principal" returns to main menu
- Visual animations work

**Steps**:
1. Start game (select "Jugar (Solo)")
2. Press Escape → pause menu should appear
3. Navigate with arrow keys → should hear hover sounds
4. Navigate to "Volver al menú principal" → text should be visible
5. Select "Volver al menú principal" → should return to main menu (not quit)

#### 5. UTF-8 Character Test
**Expected Result**:
- All Spanish text displays correctly
- No weird symbols or question marks
- Proper rendering of accented characters

**Steps**:
1. Check all menus for proper text rendering
2. Specifically verify: "Opciones", "Resolución", "meditación" (if used anywhere)
3. Look for any � symbols or boxes indicating encoding issues

#### 6. Configuration Persistence Test
**Expected Result**:
- Settings save to `config/config.json`
- Settings load correctly on restart
- No data corruption

**Steps**:
1. Change all settings in Options Menu
2. Select "Guardar"
3. Exit game completely
4. Check `config/config.json` → should contain new values
5. Restart game
6. Open Options Menu → should show saved values

### Troubleshooting

#### No Hover Sounds
- Check if `assets/sounds/hover_select.wav` exists
- Check console log for audio loading errors
- Verify system audio is working

#### Options Menu Crash
- Check console log for specific error messages
- Verify ConfigManager is properly initialized
- Check if config directory has write permissions

#### Encoding Issues
- Verify font files support UTF-8 characters
- Check that source files are saved as UTF-8
- Console logs may show encoding warnings

#### Settings Not Persisting
- Check if `config/` directory exists and is writable
- Verify `config.json` is being created/updated
- Check console logs for save/load errors

## Success Criteria

✅ **Title Enhancement**: Orange outline visible, proper positioning
✅ **Hover Sounds**: Working in all menus, no sound overlap
✅ **Options Menu**: Opens without crash, all controls functional
✅ **Settings Persistence**: Values save and load correctly
✅ **Pause Menu**: Navigation works, returns to main menu properly
✅ **UTF-8 Support**: All text renders correctly without encoding issues

## Files Modified

### Core Files
- `src/ui/MainMenu.cpp` - Title outline, UTF-8 support
- `src/ui/MainMenu.h` - No changes needed
- `src/ui/PauseMenu.cpp` - Complete rewrite with hover sounds and navigation
- `src/ui/PauseMenu.h` - Added audio members and methods
- `src/ui/OptionsMenu.cpp` - Complete simplified rewrite
- `src/ui/OptionsMenu.h` - Simplified interface
- `src/ui/UIManager.cpp` - Added triggerReturnToMainMenu method
- `src/ui/UIManager.h` - Added onReturnToMainMenu event and method
- `src/core/ConfigManager.cpp` - Enhanced volume and fullscreen support
- `src/core/ConfigManager.h` - Added new accessors and setters

### Configuration Files
- `config/config.json` - Updated structure for new volume system

### Asset Requirements
- `assets/sounds/hover_select.wav` - Required for hover sounds
- `assets/sounds/confirm.wav` - Required for confirmation sounds
- `assets/fonts/Main_font.ttf` - Should support UTF-8
- `assets/fonts/Secundary_font.ttf` - Should support UTF-8
- `assets/textures/Main Menu.jpg` - Used as background for Options Menu
