---
title: "Gameplay"
module: "gameplay"
last_updated: "2025-08-25"
authors: ["Equipo"]
tags: [status, implementation, planned-improvements]
---

# Gameplay

## 1. Resumen
Items, puzzles y reglas de juego: managers que registran objetos, gestionan interacciones, guardado y estadísticas. **MEJORAS PLANIFICADAS**: Sistemas avanzados de supervivencia incluyendo inventario restringido, combate improvisado, sistema de ruido, y mecánicas de sigilo.

## 2. Estado actual
Implementado y testeado. Soporta serialización JSON, animaciones en recolección y estadísticas. Recomendado mejorar la API de colliders para evitar re-registraciones por frame.

## 3. Mejoras Planificadas - Sistemas de Supervivencia Avanzados

### 3.1 Sistema de Inventario Restringido
**Objetivo**: Implementar gestión realista de objetos con limitaciones de peso y slots.

**Características a implementar**:
- **4 slots principales** de acceso rápido (hotbar)
- **Capacidad de peso**: 20kg máximo, afecta movimiento y fatiga
- **Gestión automática**: Intercambio entre slots principales y mochila
- **Categorización**: Diferentes tipos de items con pesos específicos
- **Persistencia**: Save/load completo del estado del inventario

**Nuevo sistema**: `src/gameplay/InventorySystem.h/.cpp`

### 3.2 Sistema de Combate Improvisado
**Descripción**: Arsenal limitado y realista para supervivencia
- **Armas improvisadas**: Tubería (+15 daño), Cuchillo (+25 daño)
- **Durabilidad**: Las armas se degradan con el uso (10-20 usos)
- **Munición escasa**: Máximo 3 proyectiles por tipo
- **Cooldown de ataque**: 1.5 segundos entre ataques melee
- **Consumo de fatiga**: 15 puntos por ataque

**Archivos a modificar**:
- `src/gameplay/Item.h/.cpp` - Agregar propiedades de armas
- `src/gameplay/CombatSystem.h/.cpp` - Nuevo sistema de combate

### 3.3 Sistema de Ruido Dinámico
**Descripción**: Cada acción genera niveles específicos de ruido que afectan la detección de enemigos.

**Niveles de ruido**:
- **Caminar normal**: 3 unidades de ruido
- **Correr**: 8 unidades de ruido
- **Abrir puertas**: 5 unidades de ruido
- **Combate melee**: 12 unidades de ruido
- **Disparar**: 15 unidades de ruido

**Integración**:
- Propagación espacial del sonido con atenuación por distancia
- IA de enemigos reacciona a fuentes de ruido
- Sistema de alerta escalonada

**Nuevo sistema**: `src/gameplay/NoiseSystem.h/.cpp`

### 3.4 Sistema de Visión Limitada
**Descripción**: Campo de visión restringido con linterna consumible.

**Características**:
- **Visión en cono**: 60° hacia adelante, alcance 150 unidades
- **Visión trasera limitada**: El jugador no puede ver completamente detrás
- **Linterna**: Consume batería (2 puntos/minuto), ilumina 200 unidades
- **Renderizado dinámico**: Fog of war fuera del campo de visión
- **Fuentes de luz**: Objetos del entorno que proporcionan iluminación

**Nuevo sistema**: `src/gameplay/VisionSystem.h/.cpp`

### 3.5 Mecánicas de Sigilo Avanzado
**Descripción**: Sistema de ocultación para evitar detección.

**Acciones de sigilo**:
- **Agacharse**: Reduce velocidad 50%, reduce ruido 70%, menor perfil visual
- **Caminar lento**: Reduce ruido 50%, mantiene velocidad controlada
- **Apagar luces**: Elimina fuentes de luz para evitar detección
- **Uso de cobertura**: Objetos del entorno ocultan al jugador

**Integración con AI**:
- Estados de sigilo afectan la detección enemiga
- Sistema de línea de visión mejorado
- Penalizaciones por romper sigilo

## 4. Implementación (Estado Actual)
- Items: `Item` y `ItemManager` con idempotencia en `interact`, notificaciones a `UIManager` y `PuzzleManager`.
- Puzzles: pasos, `PuzzleManager` y `markStepCompleted` con notificaciones y logging.
- Serialización: `saveToJson` / `loadFromJson` en managers; versionado del formato.
- Animaciones: scale & fade (500ms) al recoger ítems.

## 5. Archivos clave
- **Existentes**: `src/gameplay/Item.*`, `ItemManager.*`, `Puzzle.*`, `PuzzleManager.*`
- **Planificados**: `InventorySystem.*`, `CombatSystem.*`, `NoiseSystem.*`, `VisionSystem.*`
- Tests: `tests/gameplay/GameplayTests`

## 6. Mejoras pendientes
- **NUEVAS**: Implementar sistemas de supervivencia avanzados según planificación
- **EXISTENTES**: Implementar `CollisionManager::updateColliderBounds` para evitar re-registraciones
- **OPTIMIZACIÓN**: Considerar spatial hash para grandes cantidades de ítems

### Cambios recientes (2025-08-24)
- Se integró el `AchievementManager` para trackear progreso de recolección, supervivencia y combate
- Se colocaron 11 ítems estratégicamente en `PlayScene` y se validó la animación de recolección
- Mejoras menores en `ItemManager` para evitar re-registraciones de colisionadores

### Notas de cambios (2025-08-25)
- **PLANIFICACIÓN**: Definidos sistemas comprehensivos de supervivencia avanzada
- **ARQUITECTURA**: Diseñados sistemas de inventario, combate, ruido, visión y sigilo
- **INTEGRACIÓN**: Especificada integración con sistemas de AI, UI y persistencia

