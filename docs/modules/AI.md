---
title: "AI"
module: "ai"
last_updated: "2025-08-24"
authors: ["Equipo"]
tags: [status, implementation]
---

# AI

## 1. Resumen
Sistema de inteligencia artificial avanzado para enemigos y agentes: FSM extendida, perfiles de comportamiento, percepción multimodal, A* pathfinding y coordinación central (AIManager).

## 2. Estado actual
Implementado y operativo. Integración en `PlayScene` y `EntityManager`. Tests unitarios presentes; algunas pruebas de enlace con SFML en el runner de tests requieren ajustes en CMake.

## 3. Implementación
- Componentes clave: `AIState`, `Perception`, `Pathfinding`, `AISystem`, `AIManager`, `Enemy` (retrocompatibilidad).
- Percepción: visión en cono con checks LOS contra `CollisionManager`, detección por sonido y memoria del último punto visto.
- Pathfinding: A* grid-based con smoothing y fallback directo.
- Coordinación: AIManager coordina agentes, broadcasts de alertas y performance monitoring.

### Implementación detallada
- Estados: FSM con 10 estados (IDLE, PATROL, CHASE, ATTACK, FLEE, RETURN, INVESTIGATE, ALERT, STUNNED, DEAD) y prioridades de acción.
- Perception: cone vision, hearing, proximity, memory; usa `CollisionManager::segmentIntersectsAny()` para LOS.
- Pathfinding: A* con grid, path smoothing y cache de caminos; fallback a movimiento directo si falla.
- Integración: hooks en `Enemy` y `EntityManager`; debug rendering disponible para estados y rutas.

### Archivos clave
- `src/ai/AIState.*`, `src/ai/Perception.*`, `src/ai/Pathfinding.*`, `src/ai/AISystem.*`, `src/ai/AIManager.*`
- Tests: `tests/ai/AITests.cpp` (unit + integration scenarios).

### Tests y estado
- Tests unitarios implementados; ejecutar `cmake --build build --target AITests && ./build/tests/AITests`.
- Nota: algunos tests requieren ajustes de linkeo con SFML en el runner de tests (ver `docs/archive/ai-implementation-complete.md`).

### Issues conocidos
- Ajustar warnings float/double y fijar CMake linking para AITests.

## 4. Mejoras pendientes / To-Do
- Afinar tests que fallan (linking SFML en AITests).
- Revisar warnings de conversiones float/double en AI para limpieza de build.

## 5. Dependencias
SFML 3.x, `CollisionManager`, `EntityManager`, Google Test para tests.

## 6. Referencias
- docs/archive/ai-implementation-complete.md
- docs/archive/ai-improvement-implementation.md
- tests/ai/

## 7. Historial / Backups
Originales consolidados en `docs/backups/20250824/` (ver `changelog_docs_unify_20250824.md`).

