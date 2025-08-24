(The file `e:\Abyssal-Station\docs\modules\Collisions.md` exists, but is empty)
---
title: "Collisions"
module: "collisions"
last_updated: "2025-08-24"
authors: ["Equipo"]
tags: [status, implementation]
---

# Collisions

## 1. Resumen
Sistema de colisiones con particionamiento espacial (QuadTree / SpatialHash), eventos OnEnter/OnExit/OnStay, raycasting y soporte multi-shape. Preparado para físicas y serialización.

## 2. Estado actual
Completo, con tests mayoritariamente verdes; 2 tests de edge-case pendiente de ajuste. Integrado con `EntityManager` y utilizado por AI para checks LOS.

## 3. Implementación
- Componentes: `CollisionManager`, `CollisionSystem`, `CollisionEventManager`, `CollisionBox`.
- Soporta triggers (isTrigger), segmentIntersectsAny, raycast con RaycastHit.

### Implementación detallada
- Broad-phase: QuadTree (configurable) con SpatialHash como alternativa.
- Narrow-phase: AABB, Circle, multi-shape support y sweep tests para detección continua.
- Eventos: `OnEnter`, `OnExit`, `OnStay` con `CollisionEventManager` y callbacks.
- Debug: `CollisionDebug` visualiza colliders, spatial partition y raycasts.

### Archivos clave
- `src/collisions/CollisionManager.*`, `CollisionSystem.*`, `CollisionEventManager.*`, `CollisionBox.*`.
- Tests: `tests/collisions/CollisionTests.cpp`.

### Tests y estado
- Test coverage alta; 2 tests edge-case pendientes relacionados con QuadTree queries.

### Issues conocidos
- Revisar dos casos de test que devuelven resultados diferentes al esperado en consultas de vecinos.

## 4. Mejoras pendientes / To-Do
- Revisar QuadTree edge cases reportados por tests.

## 5. Dependencias
SFML 3.x, EntityManager, AI.

## 6. Referencias
- docs/archive/collision-improvements-implementation.md

## 7. Historial / Backups
Originales consolidados en `docs/backups/20250824/`.

### Notas de cambios (2025-08-24)
- No se realizaron cambios directos al sistema de colisiones durante esta iteración.
- Observación: se verificó la integración con el bucle de render y logging del Core; los logs ahora muestran acentos correctamente.
