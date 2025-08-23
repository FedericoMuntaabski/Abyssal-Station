---
applyTo: 'src/ai/**'
---

# Módulo: IA básica de enemigos

## Contexto
Los enemigos deben moverse en el mapa siguiendo una lógica simple de **FSM (Finite State Machine)**:
- **Idle/Patrol** → deambulan entre puntos predefinidos.
- **Chase** → si detectan al jugador en rango.
- **Attack (placeholder)** → log cuando alcanzan al jugador.
- **Return** → si pierden al jugador, vuelven a su patrulla.

## Clases a implementar
- `AIState` (enum): IDLE, PATROL, CHASE, ATTACK, RETURN.
- `Enemy`:
  - Variables: posición, velocidad, rango de visión, estado actual, puntos de patrulla.
  - Métodos: `update(float dt)`, `changeState(AIState newState)`.
- `AISystem`:
  - Actualiza todos los enemigos cada frame.
  - Gestiona transiciones de estados.
- Integración con `CollisionSystem` para evitar que atraviesen paredes.

## Restricciones
- El sistema debe ser **modular**, no hardcodear valores (usar constantes configurables).
- La lógica debe estar desacoplada de la renderización (separar IA de gráficos).
- Registrar eventos con `Logger`:  
  Ejemplo: `[AI] Enemy #id pasó de PATROL a CHASE`.

## Ejemplo esperado
- Un enemigo que patrulla entre dos puntos.
- Si el jugador entra en rango, log:  
  `[AI] Enemy 1 detectó al jugador -> CHASE`.
- Si el jugador escapa, log:  
  `[AI] Enemy 1 perdió al jugador -> RETURN`.

## Instrucciones para Copilot
- Generar FSM básica con `switch(state)`.
- Implementar función `detectPlayer()` que use distancia euclídea.
- Crear `EnemyManager` que gestione múltiples enemigos.
- Incluir parámetros editables: `speed`, `visionRange`, `attackRange`.
- Probar con 1 enemigo en el mapa inicial.
