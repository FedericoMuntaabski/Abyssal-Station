---
applyTo: "src/core/**"
---

# Core Module — Instrucciones para Copilot

## Contexto
- Este módulo gestiona la **inicialización del juego**, loop principal y renderizado base.
- Responsable de cargar **assets globales** (sprites, tiles, sonidos).
- Mantener **separación de responsabilidades**: no incluir lógica de jugador, IA o networking aquí.
- Lenguaje: C++ 2D top-down
- Librerías: SFML, ENet

## Estructura recomendada
- `Game.h / Game.cpp` → clase principal, loop principal.
- `AssetManager.h / AssetManager.cpp` → carga y manejo de sprites, tiles y sonidos.
- `Timer.h / Timer.cpp` → manejo de delta time y temporizadores.
- `Logger.h / Logger.cpp` → registro de eventos y debug.

## Patrones de diseño recomendados
- **Singleton**: AssetManager y Logger.
- **Observer / Event system**: para notificar cambios globales (ej. pausa, fin de partida).
- **Responsabilidad única**: cada clase debe tener un único propósito.

## Convenciones de código
- Separar declaraciones `.h` y definiciones `.cpp`.
- Nombres claros para clases, métodos y variables.
- Documentar métodos con comentarios de inputs, outputs y efectos secundarios.

## Git / Ramas
- Crear rama para cambios en core: `feature/core/<nombre>`.
- Commits sugeridos:
  - `[feature]` inicializar Game loop.
  - `[refactor]` optimizar AssetManager.
  - `[doc]` actualizar documentación del core.
- Antes de merge a develop/main: verificar compilación y funcionalidad básica del loop.

## Restricciones
- No implementar lógica de jugador, IA o networking en core.
- Evitar incluir assets específicos de niveles o jugadores aquí.
- Mantener independencia del módulo para que pueda reutilizarse o probarse aislado.

## Instrucciones para Copilot
- Generar nuevas clases dentro de `src/core/` siguiendo la estructura y patrones de diseño.
- Sugerir commits claros y coherentes con los cambios del módulo core.
- Recordar modularidad y separación de responsabilidades.
- Proporcionar ejemplos de inicialización del juego, render loop y manejo de assets.