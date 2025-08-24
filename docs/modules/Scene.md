---
title: "Scene"
module: "scene"
last_updated: "2025-08-24"
authors: ["Equipo"]
tags: [status, implementation]
---

# Scene

## 1. Resumen
Gestión de escenas con pila LIFO (`SceneManager`), `MenuScene`, `PlayScene` y utilidades de `SaveIntegration` para snapshot/restauración del `GameState`.

## 2. Estado actual
Operativo y probado. `SaveIntegration` realiza round-trip determinista usando setters públicos en entidades.

## 3. Implementación detallada
- `SceneManager`: push/pop/replace, delegación de eventos y render.
- `PlayScene`: orquesta managers (entities, collisions, AI, gameplay) y utiliza compute->validate->commit para movimiento.
- `SaveIntegration`: `gatherGameState` y `applyGameState` para persistencia.

## 4. Archivos clave
- `src/scene/SceneManager.*`, `PlayScene.*`, `SaveIntegration.*`.
