---
title: "Gameplay"
module: "gameplay"
last_updated: "2025-08-24"
authors: ["Equipo"]
tags: [status, implementation]
---

# Gameplay

## 1. Resumen
Items, puzzles y reglas de juego: managers que registran objetos, gestionan interacciones, guardado y estadísticas.

## 2. Estado actual
Implementado y testeado. Soporta serialización JSON, animaciones en recolección y estadísticas. Recomendado mejorar la API de colliders para evitar re-registraciones por frame.

## 3. Implementación detallada
- Items: `Item` y `ItemManager` con idempotencia en `interact`, notificaciones a `UIManager` y `PuzzleManager`.
- Puzzles: pasos, `PuzzleManager` y `markStepCompleted` con notificaciones y logging.
- Serialización: `saveToJson` / `loadFromJson` en managers; versionado del formato.
- Animaciones: scale & fade (500ms) al recoger ítems.

## 4. Archivos clave
- `src/gameplay/Item.*`, `ItemManager.*`, `Puzzle.*`, `PuzzleManager.*`.
- Tests: `tests/gameplay/GameplayTests`.

## 5. Mejoras pendientes
- Implementar `CollisionManager::updateColliderBounds` o marcar `colliderRegistered_` para evitar llamadas repetidas en `Item::update`.
- Considerar spatial hash para grandes cantidades de ítems.

### Notas de cambios (2025-08-24)
- Se confirmó la integración de notificaciones de UI (item collected) con `UIManager` y que las salidas de log ahora muestran acentos correctamente.
- El dibujo global de background fue movido a las escenas para evitar artefactos visuales cuando assets son eliminados del disco.

### Cambios recientes (2025-08-24) — delta

- Se integró el `AchievementManager` para trackear progreso de recolección, supervivencia y combate; notificaciones automáticas en UI al desbloquear logros.
- Se colocaron 11 ítems estratégicamente en `PlayScene` y se validó la animación de recolección y la notificación por cada pickup.
- Mejoras menores en `ItemManager` para evitar re-registraciones de colisionadores por frame y prevenir posibles double-collect.

