---
title: "Entities"
module: "entities"
last_updated: "2025-08-24"
authors: ["Equipo"]
tags: [status, implementation]
---

# Entities

## 1. Resumen
Gestión de entidades (Entity, EntityManager, Player, MovementHelper). Añadidos setters públicos para restauración y telemetría opcional.

## 2. Estado actual
Completo y probado. Integración con collision/scene/save. Recomendado exponer `Puzzle::forceComplete()` para restauración completa.

## 3. Referencias
- docs/archive/entities-improvements-implementation.md
- docs/archive/entities-status.md

### Implementación detallada
- `Player`: setters públicos (`setPosition`, `setHealth`) y API compute/commit para movimiento seguro.
- `MovementHelper`: swept-AABB, slide/bounce modes y `CollisionResult` con detalles de impacto.
- `EntityManager`: batch operations, performance stats y frustum culling.

### Archivos clave
- `src/entities/Player.*`, `MovementHelper.*`, `EntityManager.*`, `EntityDebug.*`.
- Tests: `tests/entities/EntityTests.*` (round-trip save/load, performance tests).

### Issues conocidos
- `Puzzle::forceComplete()` recomendado para restauraciones de puzzles durante load.
