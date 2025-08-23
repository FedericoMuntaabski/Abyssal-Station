---
applyTo: 'src/entities/**'
---

# 📦 Módulo: Entities / Player

## Objetivo
Definir el sistema de entidades del juego, comenzando con `Player`.  
Debe integrarse con **Core** (Logger, Config), **SceneManager** y **InputManager**.  
El Player servirá como base para futuros NPCs y entidades controlables.

## Requisitos
- C++17, estilo modular.
- Separar header (`.h`) y fuente (`.cpp`).
- Usar forward declarations cuando sea posible para reducir dependencias.
- Preparar Player para extensión futura (enemigos, NPCs).

## Clases principales
- **Entity (abstracta)**
  - Atributos básicos: id, posición (sf::Vector2f), tamaño, sprite/shape.
  - Métodos virtuales: update(deltaTime), render(sf::RenderWindow&).
- **Player (hereda de Entity)**
  - Atributos: velocidad, vida, estado actual (Idle, Walking, Dead).
  - Métodos: handleInput(), update(), render().
  - Interacción con InputManager para moverse.
- **EntityManager**
  - Contenedor de entidades activas.
  - Métodos: addEntity(), removeEntity(), updateAll(), renderAll().

## Integración
- El Player debe inicializarse en `PlayScene`.
- InputManager controla el movimiento (WASD).
- Logger informa creación y destrucción de entidades.
- Escenas usan EntityManager para gestionar todo lo que existe en pantalla.

## Git Flow
- Rama: `feature/entities-player`
- Commits:
  - `[feature] Definir clase abstracta Entity`
  - `[feature] Implementar Player con movimiento básico`
  - `[feature] Crear EntityManager y registrar Player`
  - `[fix] Ajustar colisión Player con límites de ventana`

## Instrucciones para Copilot
- Sugerir herencia clara (Entity -> Player).
- Proponer EntityManager eficiente (usar vector y punteros únicos).
- Generar ejemplos de Player respondiendo a InputManager.
- Recordar separación en headers/cpp.
- Preparar comentarios `TODO` para colisiones y animaciones futuras.
