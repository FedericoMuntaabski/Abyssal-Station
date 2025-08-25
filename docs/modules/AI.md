---
title: "AI"
module: "ai"
last_updated: "2025-08-25"
authors: ["Equipo"]
tags: [status, implementation, planned-improvements]
---

# AI

## 1. Resumen
Sistema de inteligencia artificial avanzado para enemigos y agentes: FSM extendida, perfiles de comportamiento, percepción multimodal, A* pathfinding y coordinación central (AIManager). **MEJORAS PLANIFICADAS**: Sistema de enemigos avanzado para supervivencia/horror con detección sensorial múltiple, persistencia inteligente y acecho psicológico.

## 2. Estado actual
Implementado y operativo. Integración en `PlayScene` y `EntityManager`. Tests unitarios presentes; algunas pruebas de enlace con SFML en el runner de tests requieren ajustes en CMake.

## 3. Mejoras Planificadas - Sistema de Enemigos Avanzado

### 3.1 IA Reactiva Mejorada
**Objetivo**: Comportamientos inteligentes basados en estímulos del entorno para aumentar immersión.

**Características a implementar**:
- **Patrullaje dinámico**: Rutas variables, no predecibles
- **Investigación de ruidos**: Se dirigen al último punto de ruido conocido
- **Persecución inteligente**: Predicen movimiento del jugador
- **Memoria de ubicaciones**: Recuerdan donde vieron al jugador por última vez

**Archivos a modificar**:
- `src/ai/AIState.h/.cpp` - Nuevos estados INVESTIGATE_NOISE, PREDICT_MOVEMENT
- `src/ai/Perception.h/.cpp` - Mejoras en detección sensorial

### 3.2 Ataques Físicos y Psicológicos
**Descripción**: Daño diversificado que afecta múltiples aspectos del jugador
- **Ataque físico**: 20-35 de daño directo
- **Grito intimidante**: +25 fatiga, efecto de pantalla temblorosa
- **Presencia amenazante**: Drenaje pasivo de 1 batería/segundo en proximidad
- **Ataques de emboscada**: Doble daño si atacan desde la oscuridad

### 3.3 Detección Sensorial Múltiple
**Descripción**: Sistema sensorial complejo y realista
- **Visión limitada**: Cono de 45°, alcance 120 unidades
- **Audición sensible**: Detecta ruido en radio de 200 unidades
- **Detección de vibraciones**: Percibe movimiento en suelo en 50 unidades
- **Memoria sensorial**: Mantienen alerta 30 segundos después de perder contacto

### 3.4 Sistema de Persistencia Inteligente
**Descripción**: Enemigos que no se rinden fácilmente
- **Búsqueda activa**: Continúan buscando 5 minutos después de perder rastro
- **Puntos de interés**: Revisan lugares donde encontraron al jugador antes
- **Comunicación entre enemigos**: Comparten información de ubicación
- **Escalada de alerta**: Más enemigos se activan si uno encuentra al jugador

### 3.5 Modo Acecho Psicológico
**Descripción**: Tensión psicológica mediante seguimiento silencioso
- **Seguimiento a distancia**: Mantienen 100 unidades de distancia
- **Apariciones breves**: Se muestran brevemente en la periferia de la visión
- **Sonidos ambientales**: Generan ruidos sutiles para crear tensión
- **Ataque sorpresa**: Después de 60 segundos de acecho, atacan con ventaja

### 3.6 Sistema de Recompensa de Riesgo
**Descripción**: Incentivos equilibrados para enfrentar o evitar enemigos
- **Derrotar enemigos**: +1 batería extra permanente (máximo +5)
- **Objetos valiosos**: Enemigos portan llaves o recursos únicos
- **Zonas de alto riesgo**: Más enemigos = mejores recompensas
- **Penalización por huida**: Enemigos se vuelven más agresivos si huyes repetidamente

## 4. Implementación (Estado Actual)
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

## 5. Mejoras pendientes / To-Do
- **NUEVAS**: Implementar sistema de enemigos avanzado según planificación
- **EXISTENTES**: Afinar tests que fallan (linking SFML en AITests)
- **CLEANUP**: Revisar warnings de conversiones float/double en AI para limpieza de build

## 6. Dependencias
SFML 3.x, `CollisionManager`, `EntityManager`, Google Test para tests, **NUEVAS**: `NoiseSystem`, `VisionSystem`.

## 7. Referencias
- docs/archive/ai-implementation-complete.md
- docs/archive/ai-improvement-implementation.md
- tests/ai/

### Notas de cambios (2025-08-25)
- **PLANIFICACIÓN**: Definidas mejoras comprehensivas para sistema de enemigos avanzado
- **ARQUITECTURA**: Diseñado sistema de detección sensorial múltiple y persistencia inteligente
- **GAMEPLAY**: Especificadas mecánicas de acecho psicológico y recompensa de riesgo

## 7. Historial / Backups
Originales consolidados en `docs/backups/20250824/` (ver `changelog_docs_unify_20250824.md`).

### Notas de cambios (2025-08-24)
- Se verificó la salida de logs de IA y se confirmó que los caracteres acentuados ahora se muestran correctamente tras cambios en el subsistema de Logger en Core.
 

