---
applyTo: 'src/ui/**'
---

# Módulo: UI / Menus

## Objetivo
Implementar un sistema modular de menús y UI que permita:
- Menú principal
- Menú de pausa (Escape)
- Menú de opciones
- Navegación con teclado (InputManager)
- Integración con SceneManager y Game

## Estructura de archivos
- `src/ui/Menu.h / Menu.cpp` → Clase base abstracta de menú
- `src/ui/MainMenu.h / MainMenu.cpp`
- `src/ui/PauseMenu.h / PauseMenu.cpp`
- `src/ui/OptionsMenu.h / OptionsMenu.cpp`
- `src/ui/UIManager.h / UIManager.cpp` → Opcional, gestiona menús activos

## Requisitos de diseño
1. **Clase Menu (abstracta)**
   - Métodos virtuales: `handleInput()`, `update(float dt)`, `render(sf::RenderWindow&)`
   - Métodos opcionales: `onEnter()`, `onExit()`
   - Permitir activar/desactivar menú

2. **Clases concretas**
   - `MainMenu`: opciones Start Game, Options, Exit
   - `PauseMenu`: Resume, Options, Quit
   - `OptionsMenu`: Volumen, Controles (placeholder)

3. **UIManager**
   - Mantiene una pila o vector de menús activos
   - Delegar `update` y `render` a los menús activos
   - Permite abrir/cerrar menús y cambiar entre ellos

4. **Integración con InputManager**
   - Navegación: flechas arriba/abajo, Enter, Escape
   - Selección de opción activa ejecuta acciones (ej. cambiar escena, cerrar menú)

5. **Logging y debug**
   - Registrar apertura/cierre de menús con `Logger`
   - Opcional: resaltar opción activa en consola para debug

## Buenas prácticas
- Mantener menús autocontenidos
- Separar lógica de renderizado de lógica de acciones
- Evitar hardcodear posiciones y textos: usar constantes o config

## Instrucciones para Copilot
- Generar clase base `Menu` y las clases derivadas
- Implementar `UIManager` con pila de menús
- Proponer navegación con InputManager
- Mostrar ejemplo de MainMenu activo al iniciar el juego
- Incluir logs de apertura/cierre de menús
