# UI module status

This document summarizes the UI/menu work in the `feature/UI` branch and the concrete changes made under `src/ui` and related integration points. It was updated to reflect recent additions: mouse support, arrow-key navigation, hover animation, and a basic controls remapping UI.

## High-level summary

Implemented a modular menu system with:
- An abstract `Menu` base class.
- Concrete menus: `MainMenu`, `PauseMenu`, `OptionsMenu`.
- A `UIManager` that owns menus and acts as a simple stack.
- Integration with `SceneManager` and `Game` so menus are visible and reactive at runtime.
- Input handling improvements in `InputManager` to correctly detect per-frame "just pressed" events and support multiple bindings.

The menus are implemented as simple, self-contained classes that handle their own input, rendering and lifecycle hooks. Logging via `core::Logger` is used to record open/close and selection events.

## Files added / changed (UI folder)

- Added
  - `src/ui/Menu.h` / `src/ui/Menu.cpp` — abstract base class providing:
    - Pure virtual: `handleInput()`, `update(float)`, `render(sf::RenderWindow&)`.
    - Optional lifecycle hooks: `onEnter()`, `onExit()`.
    - Activation control: `activate()`, `deactivate()`, `isActive()`, `name()`.
  - `src/ui/MainMenu.h` / `src/ui/MainMenu.cpp` — main menu with options: `Start Game`, `Options`, `Exit`.
  - Navigation with MoveUp/MoveDown (W/S and arrow keys supported), selection with Confirm (Enter or mouse left click).
  - Mouse hover changes the highlighted option and left-click confirms the selected option.
  - Added a subtle hover animation (scale-up) for the highlighted option for clearer feedback.
  - Integrates with `SceneManager` to push `PlayScene` when "Start Game" is chosen and opens `OptionsMenu` via `UIManager`.
  - `src/ui/PauseMenu.h` / `src/ui/PauseMenu.cpp` — pause menu with options: `Resume`, `Options`, `Quit`.
    - Toggleable from `PlayScene` via Escape/Pause.
    - When entered, sets `UIManager::setPaused(true)` and clears it on exit.
    - Opens `OptionsMenu` via `UIManager` when selected.
  - `src/ui/OptionsMenu.h` / `src/ui/OptionsMenu.cpp` — options placeholder with: `Volume`, `Controls`, `Back`.
  - Left/Right adjust volume (simple integer 0..100) and logs changes.
  - Controls panel now displays current binding names (first binding shown) and supports a basic remapping flow: selecting Controls and confirming enters remap mode and the next key or mouse button pressed will rebind the targeted action.
  - `src/ui/UIManager.h` / `src/ui/UIManager.cpp` — owns menus (stack), delegates `update`/`render`, logs push/pop/clear.
    - API: `pushMenu(Menu*)`, `popMenu()`, `currentMenu()`, `update(dt)`, `render(window)`, `clear()`.
    - Query helpers added: `isMenuActive(name)`, `isAnyMenuActive()` and pause flag `setPaused(bool)/isPaused()`.

- Modified (integration)
  - `src/scene/MenuScene.cpp` / `.h`: creates a `UIManager` and pushes `MainMenu` on `onEnter`; delegates `update`/`render` to `UIManager`.
  - `src/scene/PlayScene.cpp` / `.h`: owns a `UIManager`, pushes `PauseMenu` when pressing Escape (toggle), delegates menu `update`/`render` and:
    - Freezes the game logic while `PauseMenu` is active (no player movement, no AI updates).
    - Avoids input propagation to game while any menu is active (menus take priority).
  - Shows an interaction hint when the player is close to an item: an icon sprite (fallback: text via `core::FontHelper` or a pulsing circle). Pressing the `Interact` action (default `E`) triggers the interaction flow handled by `gameplay::ItemManager` and the UI shows a toast with the result.
  - `src/core/Game.cpp`: calls `InputManager::endFrame()` once per frame (after event processing) so "just pressed" semantics work.
  - `CMakeLists.txt`: added UI source files so they are compiled and linked.

- Input changes (important for menu navigation)
  - `src/input/InputManager.h` / `.cpp`:
  - `bindKey` appends a key (supports multiple key bindings per action) and arrow keys were added by default.
  - Added mouse button support (bindMouse) and default: left mouse -> Confirm.
  - Added `endFrame()` to snapshot previous state once per frame; `update(event)` updates current state.
  - `isActionJustPressed`/`isActionPressed`/`isActionReleased` check all bound keys and mouse buttons for an action.
  - New APIs for remapping and UI: `rebindKeys`, `rebindMouse`, `getLastKeyEvent`, `getLastMouseButtonEvent`, `clearLastEvents`, `getBindingName` and getters to expose bindings for display.
    - `Action::Interact` was added and bound by default to `E`; the UI uses `getBindingName(Action::Interact)` to render the interaction label/icon near items.

## Behavior & how it works now

- Menu navigation works with keyboard input (MoveUp/MoveDown and Confirm). `isActionJustPressed` now returns true once per frame for newly-pressed keys because `endFrame()` is called once per frame by the game loop.
 - Menu navigation works with keyboard input (MoveUp/MoveDown and Confirm) and arrow keys are supported.
 - Mouse: hovering over menu items highlights them; left click acts as Confirm.
 - `isActionJustPressed` semantics were corrected by snapshotting previous input state once per frame; the game calls `InputManager::endFrame()` before processing events so just-pressed/just-released queries behave consistently during event handling and per-frame updates.
- The `MainMenu` is shown via `MenuScene` at game start (the `Game` pushes `MenuScene` on startup; `MenuScene` pushes `MainMenu`).
- `PlayScene` handles Pause toggling: pressing Escape will push/pop `PauseMenu` through the per-scene `UIManager`.
- When `PauseMenu` is active the `PlayScene::update()` will early-return (game logic frozen). When the menu is closed the game resumes.
- `OptionsMenu` is usable from both `MainMenu` and `PauseMenu` and adjusts a volume integer while logging changes. Real audio integration is left as a TODO.

## Verification performed

- Built the full project after all changes: `cmake --build build --config Release` (MSBuild on Windows) — build completed successfully and produced `build/Release/AbyssalStation.exe`.
- Manual checks during development:
 - Built the full project after all changes: `cmake --build build --config Release` (MSBuild on Windows) — build completed successfully and produced `build/Release/AbyssalStation.exe`.
 - Manual checks during development:
   - Verified keyboard navigation (W/S and arrow keys) and left-click Confirm.
   - Confirmed hover highlights and hover animation in `MainMenu`.
   - Verified basic remap flow in `OptionsMenu` (press Controls -> confirm -> press a key or click mouse button to rebind).
  - Verified interaction hint appears near items and pressing `E` picks up items and shows a toast.
  - Verified `OptionsMenu` remap flow and that `getBindingName` is used to display the currently-bound key in UI.

## How to try it locally

From the workspace root on Windows PowerShell:

```powershell
# Configure and build (if not already configured)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
````markdown
# UI module status (updated)

Small, focused update that reflects the current `src/ui` implementation and its integration points (input remapping, mouse support, menu stack, and persistence helpers).

Checklist of changes covered here
- [x] Files present in `src/ui` and responsibilities.
- [x] Menu base (`Menu`) and concrete menus: `MainMenu`, `PauseMenu`, `OptionsMenu`.
- [x] `UIManager` behavior: stack, delegation, pause flag and query helpers.
- [x] Navigation: keyboard, arrows, mouse hover/click and remap flow.
- [x] Integration notes: `SceneManager`, `PlayScene`, `ConfigManager`/`SaveManager` and `SaveIntegration`.

High-level summary

The UI package implements a modular menu system suitable for keyboard and mouse. Menus are self-contained classes that handle input, drawing and lifecycle. The `UIManager` keeps an ordered stack of active menus and provides helpers (isAnyMenuActive/isMenuActive) and a pause flag used by `PlayScene`.

Files in `src/ui`
- `Menu.h` / `Menu.cpp` — abstract base class with `handleInput()`, `update(float)`, `render(sf::RenderWindow&)`, optional `onEnter()`/`onExit()` and activation helpers.
- `MainMenu.h` / `MainMenu.cpp` — Start / Options / Exit, keyboard + mouse navigation, integrates with `SceneManager` to start the game or open the options.
- `PauseMenu.h` / `PauseMenu.cpp` — Resume / Options / Quit, toggled from `PlayScene` (Escape). Sets `UIManager` pause flag while active.
- `OptionsMenu.h` / `OptionsMenu.cpp` — Volume (left/right), Controls (remap flow), Back. Shows binding names using `InputManager::getBindingName`.
- `Menu.cpp` / `Menu.h` — generic menu helpers (selection index, rendering helpers).
- `UIManager.h` / `UIManager.cpp` — owns menus, stack semantics, delegates update/render/handleInput, logs push/pop/replace and exposes pause state and helpers.

Navigation and remapping
- Keyboard: MoveUp/MoveDown (W/S and arrow keys default), Confirm = Enter.
- Mouse: hovering highlights items; left-click acts as Confirm.
- Input remapping: `OptionsMenu` supports a simple remap flow — selecting Controls enters remap mode; the next key or mouse button event replaces the binding for the chosen action. The `InputManager` exposes remapping helpers (getLastKeyEvent, rebindKey, getBindingName) used by the UI.

Integration with scenes and game loop
- `MenuScene` creates/initializes a `UIManager` and pushes the `MainMenu` on enter. `Game::run()` pushes `MenuScene` on startup.
- `PlayScene` owns a per-scene `UIManager` instance. While a menu is active the scene avoids running game logic (AI, movement) — menus take input priority.

Persistence and integration with config/save systems
- `OptionsMenu` currently adjusts a volume integer and remaps controls. These values are intended to be saved/loaded by `core::ConfigManager` (config/config.json). The typical flow should be:
  - On startup: `core::ConfigManager::loadConfig()` -> `applyConfig(UIManager&)` (applies UI-related settings such as notification durations, defaults).
  - When options change in `OptionsMenu`: call `ConfigManager::saveConfig()` to persist user choices (volume, language, key bindings).
- Save/Load of game state is outside `src/ui` but menus are the natural place to trigger it: use `scene::gatherGameState(...)` (from `SaveIntegration`) and `core::SaveManager::saveGame(...)` to persist, and reverse the flow to load.

Logging and observable messages
- Menus emit logs via `core::Logger` for open/close/selection events.
- Integration messages from core:
  - `[config] Usando valores por defecto; no se encontró: <ruta>` — when config file is absent.
  - `[save] Archivo no encontrado: <ruta>` — when a requested save file is missing.

Verification & known limitations
- The UI compiles and links in the project. Keyboard and mouse navigation, hover, hover animation and remap flow work at the API/manual level.
- Known TODOs:
  - Wire `OptionsMenu::applyVolume()` to the real audio system (AssetManager / AudioManager).
  - Consider `UIManager::pushMenu(std::unique_ptr<Menu>)` to avoid raw-pointer ownership.
  - Improve remap UI to display multiple bindings and provide cancel/timeout during remap.
  - Add a Save / Load menu item (MainMenu or Options) that calls `SaveIntegration` + `SaveManager` for full round-trip persistence.

Next suggested actions
- Implement `UIManager` overloads that accept `std::unique_ptr<Menu>` and update call sites.
- Wire `OptionsMenu` volume changes to an audio manager and persist via `ConfigManager`.
- Add menu entries for Save/Load that use `scene::gatherGameState` + `SaveManager::saveGame` and `SaveManager::loadGame` + `scene::applyGameState`.
- Add unit tests for `UIManager` activation and a small integration test for remap flow.

If you want I can implement one of the next steps above; tell me which one to prioritize and I will make the change and run a build/test.

````
- wire `OptionsMenu` volume to actual audio, or
