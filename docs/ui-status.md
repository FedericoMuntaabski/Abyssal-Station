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
cmake --build build --config Release

# Run the built executable (Release)
.
"build\Release\AbyssalStation.exe"
```

Controls (default bindings)
- MoveUp: W
- MoveDown: S
- MoveLeft: A
- MoveRight: D
- Confirm: Enter
- Cancel: Escape
- Pause: P (Pause toggling was wired to Escape in PlayScene; P is available as Pause key as well)

While running:
- Main menu appears at start; navigate with W/S (up/down) and Enter to select.
- Start Game -> enters PlayScene.
- Press Escape in PlayScene to open PauseMenu — the game logic will freeze while the pause menu is active.
- From PauseMenu you can select Options to open the OptionsMenu (adjust Volume with Left/Right) or Resume to return to the game.

## Mapping to original requirements

- Menu base class with virtual `handleInput`, `update`, `render`, plus `onEnter`/`onExit` and activation control — Done.
- `MainMenu` with Start/Options/Exit, keyboard navigation and integration with `SceneManager` — Done.
- `PauseMenu` with Resume/Options/Quit, toggled by Escape, integrated with scene — Done (Pause now freezes PlayScene logic).
- `OptionsMenu` with Volume/Controls placeholders and callbacks for volume — Done (volume changes logged; integration with audio is a TODO).
- `UIManager` with stack semantics, delegation to active menus and logging — Done.
- Integration with `InputManager` for navigation and per-frame detection — Done (added `endFrame()` and multi-binding support).
- Logging of menu open/close and selections — Done (uses `core::Logger`).

## Known limitations & TODOs

- `OptionsMenu::applyVolume()` only logs the new value. Hook it to the audio system to actually change game volume.
- `UIManager::pushMenu(Menu*)` currently takes a raw pointer and takes ownership. Consider overloading to accept `std::unique_ptr<Menu>` to avoid mistakes.
- Menu visuals are minimal text-only render; layout, alignment, and focus visuals can be improved.
- Menu visuals are minimal text-only render; layout, alignment, and focus visuals can be improved.
- Basic mouse support is implemented (hover + left click confirm); improvements remain to make the mouse UX richer (scroll, tooltips).
- Input binding presentation is implemented at a basic level: `OptionsMenu` shows the first binding and supports remap flow, but it should be improved to list multiple bindings and show nicer labels for special keys.
 - The controls remap UI is intentionally minimal: it replaces the selected action's binding with the next raw key or mouse button event. Improvements to implement:
  - full per-action selection and remapping UI (navigate action list and remap the selected action),
  - show multiple bindings per action and more readable names for special keys,
  - add confirmation/cancel during remap and a timeout to abort.

Note: The `getenv` usage in `MenuScene` was replaced with `_dupenv_s` to avoid MSVC deprecation warnings.

Notes
- `core::FontHelper` is used by menus and `PlayScene` as a central fallback for font loading (helps keep consistent fallbacks between UI and scenes).

## Next suggested steps

1. Wire `OptionsMenu::applyVolume()` to the audio system (e.g., set global music/sfx volume in `AssetManager` or audio manager).
2. Implement a safer `UIManager::pushMenu(std::unique_ptr<Menu>)` overload and convert call sites to use `unique_ptr`.
3. Improve controls remapping UI: full per-action selection, show multiple bindings and readable names, cancel/timeout, and better visual feedback while remapping.
4. Improve menu rendering (font sizes, centered layout, translucent background when paused).
5. Add small unit tests for `UIManager` and menu activation logic.

---

If you want, I can now:
- implement `pushMenu(std::unique_ptr<Menu>)` overloads and convert call sites, or
- wire `OptionsMenu` volume to actual audio, or
- add a translucent overlay when paused and center the menus.

Tell me which of those you prefer next and I will implement it.
