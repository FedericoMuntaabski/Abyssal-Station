---
title: "Input"
module: "input"
last_updated: "2025-08-24"
authors: ["Equipo"]
tags: [status, implementation]
---

# Input

## 1. Resumen
Módulo de entrada que maneja teclado y ratón, bindings multi-bind, serialización de bindings y remapeo en runtime.

## 2. Estado actual
Completo y probado: remapeo por `OptionsMenu` funciona y persiste en `config/input_bindings.json`. Tests unitarios incluidos.

## 3. Implementación detallada
- Serialización JSON con `nlohmann::json` para `saveBindings` / `loadBindings`.
- Runtime remap: `getLastKeyEvent` y `getLastMouseButtonEvent` usados por UI para reasignar.
- Extras: input history (event timestamps), action caching (precompute per frame) y multi-bind support.

## 4. Archivos clave
- `src/input/InputManager.*`, `src/ui/OptionsMenu.*` (integration).
- Tests: `tests/input/InputManagerTest.cpp`.

## 5. Mejoras pendientes
- Añadir soporte básico de gamepad y mejorar `keyToString` para combinaciones.

### Notas de cambios (2025-08-24)
- `OptionsMenu` implementar remapeo y persistencia; se verificó que la escritura de `config/input_bindings.json` funciona.
- Se comprobó que los logs relacionados con input muestran correctamente acentos tras la mejora del Logger.

