---
title: "UI"
module: "ui"
last_updated: "2025-08-24"
authors: ["Equipo"]
tags: [status, implementation]
---

# UI

## 1. Resumen
Subsistema de interfaz: `UIManager`, `MainMenu`, `OptionsMenu`, `InputHelper`, y sistema de `Toast` para notificaciones.

## 2. Estado actual
Funcional; permite remapeo en runtime y persistencia de opciones. Falta iconografía y soporte de combinaciones avanzadas.

## 3. Implementación detallada
- `UIManager`: gestión de widgets, toasts y animaciones; `FontHelper` para caché de fuentes.
- `OptionsMenu`: remapeo integrándose con `InputManager` y guardado inmediato de bindings.
- `InputHelper`: formatea bindings y detecta dispositivo activo.

## 4. Archivos clave
- `src/ui/UIManager.*`, `MainMenu.*`, `OptionsMenu.*`, `InputHelper.*`.
