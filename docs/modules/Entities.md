---
title: "Entities"
module: "entities"
last_updated: "2025-08-25"
authors: ["Equipo"]
tags: [status, implementation, planned-improvements]
---

# Entities

## 1. Resumen
Gestión de entidades (Entity, EntityManager, Player, MovementHelper). Añadidos setters públicos para restauración y telemetría opcional. **MEJORAS PLANIFICADAS**: Sistema avanzado de supervivencia para Player con estados vitales, inventario restringido, y mecánicas de sigilo.

## 2. Estado actual
Completo y probado. Integración con collision/scene/save. Recomendado exponer `Puzzle::forceComplete()` para restauración completa.

## 3. Mejoras Planificadas - Sistema de Supervivencia Avanzado

### 3.1 Sistema de Estados Vitales del Player
**Objetivo**: Implementar múltiples recursos vitales interconectados que afecten el gameplay de supervivencia/horror.

**Estados a implementar**:
- **Vida (Health)**: 0-100, muerte permanente al llegar a 0
- **Batería (Battery)**: 0-100, recarga lenta automática (1 punto/3 segundos)
- **Fatiga (Fatigue)**: 0-100, aumenta con acciones físicas
- **Peso (Weight)**: Calculado dinámicamente según inventario

**Archivos a modificar**:
- `src/entities/Player.h/.cpp` - Agregar variables de estado y lógica
- `src/entities/PlayerController.h/.cpp` - Control avanzado de estados
- Integración con `UIManager` para HUD de estados vitales

### 3.2 Sistema de Estamina Limitada
**Descripción**: Gestión de energía que limita acciones del jugador
- Correr drena fatiga (5 puntos/segundo)
- Atacar drena fatiga (15 puntos por ataque)
- Peso afecta velocidad de drenaje (+1 punto/segundo por cada 10kg extra)
- Recuperación: 2 puntos/segundo en reposo

### 3.3 Inventario Restringido por Peso y Slots
**Descripción**: Sistema de inventario limitado y realista
- **4 slots principales** de acceso rápido (hotbar)
- **Capacidad de peso**: 20kg máximo
- **Gestión de slots**: Intercambio automático entre slots principales y mochila
- **Peso por item**: Varía según tipo de objeto

**Nuevo sistema**: `src/gameplay/InventorySystem.h/.cpp`

### 3.4 Sistema de Visión y Linterna
**Descripción**: Campo de visión limitado con consumo de batería
- **Visión en cono**: 60° hacia adelante, alcance 150 unidades
- **Sin visión trasera**: El jugador no puede ver detrás
- **Linterna consume batería**: 2 puntos/minuto cuando está encendida
- **Oscuridad dinámica**: Renderizado basado en fuentes de luz

### 3.5 Sistema de Sigilo Avanzado
**Mecánicas de ocultación**:
- **Agacharse**: Reduce velocidad 50%, reduce ruido 70%
- **Caminar lento**: Reduce ruido 50%
- **Apagar luces**: Evita detección visual de enemigos
- **Cobertura**: Usar objetos del entorno para ocultarse

### 3.6 Sistema de Heridas Persistentes
**Consecuencias duraderas por daño severo**:
- **Umbral crítico**: Si la vida baja del 35%, se activan penalizaciones
- **Cojera**: Velocidad reducida 25% permanentemente
- **Pérdida de slots**: Solo 2 slots principales disponibles
- **Regeneración lenta**: 1 punto de vida cada 30 segundos hasta 50% máximo

## 4. Referencias
- docs/archive/entities-improvements-implementation.md
- docs/archive/entities-status.md

### Implementación detallada (Estado Actual)
- `Player`: setters públicos (`setPosition`, `setHealth`) y API compute/commit para movimiento seguro.
- `MovementHelper`: swept-AABB, slide/bounce modes y `CollisionResult` con detalles de impacto.
- `EntityManager`: batch operations, performance stats y frustum culling.

### Archivos clave
- `src/entities/Player.*`, `MovementHelper.*`, `EntityManager.*`, `EntityDebug.*`.
- Tests: `tests/entities/EntityTests.*` (round-trip save/load, performance tests).

### Issues conocidos
- `Puzzle::forceComplete()` recomendado para restauraciones de puzzles durante load.

### Notas de cambios (2025-08-25)
- **PLANIFICACIÓN**: Definidas mejoras comprehensivas para sistema de supervivencia avanzado del Player
- **ARQUITECTURA**: Diseñado sistema de estados vitales interconectados
- **GAMEPLAY**: Especificadas mecánicas de inventario, sigilo, y heridas persistentes
