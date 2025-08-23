
# Estado actual del módulo `scene` (actualizado)

Resumen corto
- Objetivo: Documentar el estado actual de `src/scene` después de las últimas iteraciones: gestor de escenas, `PlayScene` integrado con `EntityManager`, `CollisionManager` y lógica de pre-move para el `Player`.
- Estado general: Implementación funcional y probada localmente. `MenuScene` y `PlayScene` operan y las transiciones funcionan; `PlayScene` ahora coordina input, pre-validación de movimiento y resolución de colisiones.

Checklist de adaptación
- [x] Listar archivos y responsabilidades en `src/scene`.
- [x] Describir la API pública de `Scene` y `SceneManager`.
- [x] Detallar comportamiento de `MenuScene` y `PlayScene`, incluyendo integración con `InputManager`, `EntityManager` y `CollisionManager`.
- [x] Documentar la integración con `Game`, assets y build.

Archivos relevantes
- `src/scene/Scene.h` (interfaz base)
- `src/scene/SceneManager.h` / `src/scene/SceneManager.cpp`
- `src/scene/MenuScene.h` / `src/scene/MenuScene.cpp`
- `src/scene/PlayScene.h` / `src/scene/PlayScene.cpp` (actualizado para colisiones y pre-move)

Visión general
El módulo `scene` sigue proveyendo la abstracción de pantallas con una pila LIFO. `PlayScene` fue extendido para crear y orquestar un `CollisionManager` y `CollisionSystem`, inyectarlos en `EntityManager`, y usar una estrategia de pre-validación de movimiento antes de aplicar cambios de posición al `Player`.

1) `Scene` (interfaz)
- Ubicación: `src/scene/Scene.h`.
- Contrato resumido:
  - virtual ~Scene() = default;
  - virtual void handleEvent(sf::Event& event) = 0;
  - virtual void update(float dt) = 0;
  - virtual void render(sf::RenderWindow& window) = 0;
  - virtual void onEnter() {}
  - virtual void onExit() {}

2) `SceneManager`
- Ubicación: `src/scene/SceneManager.{h,cpp}`.
- Comportamiento: mantiene la pila de escenas, delega eventos y gestión de ciclo de vida (igual que antes).

3) `MenuScene`
- Ubicación: `src/scene/MenuScene.*`.
- Comportamiento: igual que antes; busca fuente en `assets/fonts`, muestra texto y permite transicionar a `PlayScene`.

4) `PlayScene` (novedades y flujo)
- Ubicación: `src/scene/PlayScene.*`.
- Componentes inicializados en `onEnter()`:
  - `entities::EntityManager` (gestor de entidades)
  - `collisions::CollisionManager` (registro de colliders)
  - `collisions::CollisionSystem` (resolución AABB + logging)
  - `EntityManager::setCollisionManager(...)` se llama para que `EntityManager` registre/actualice colliders automáticamente.
  - Se crea un `Player` (id 1) y una `Wall` estática (id 2). La pared está colocada actualmente en `(480,170)` con tamaño `(50,150)` para pruebas.

- Flujo principal en `update(dt)`:
  1. Leer entrada y actualizar `m_velocity` / llamar `m_player->handleInput(...)`.
  2. Llamar `EntityManager::updateAll(dt)` — esto actualiza estados y sincroniza collider bounds en `CollisionManager`.
  3. Calcular posición objetivo del player con `player->computeIntendedMove(dt)`.
  4. Crear `sf::FloatRect` con la posición objetivo y consultar `CollisionManager::firstColliderForBounds(bounds, player)`:
     - Si no hay colisión, llamar `player->commitMove(intended)`.
     - Si hay un bloqueador, registrar mensaje y no aplicar movimiento (bloqueo completo por ahora).
  5. Llamar `CollisionSystem::resolve(player, dt)` como fallback para corregir solapamientos residuales.
  6. Sincronizar debug visuals con `player->position()`.

  5) Integración con AI / EnemyManager (novedades)
  - `PlayScene::onEnter()` ahora crea también un `ai::EnemyManager` y registra los enemigos que crea. El flujo de la escena mantiene la propiedad de las entidades (a través de `EntityManager`) y pasa punteros no propietarios a `EnemyManager` para coordinar la IA.
  - Los enemigos (ej. id=3 y id=4) se crean en `onEnter()` y se agregan a `EntityManager`. Inmediatamente después se registran en `EnemyManager` con `addEnemyPointer`.
  - En `PlayScene::update(dt)` el manejo de enemigos fue centralizado:
    - `m_enemyManager->updateAll(dt, playerPos)` ejecuta las FSMs.
    - `m_enemyManager->planAllMovement(dt, m_collisionManager.get())` planifica movimientos collision-aware para cada enemigo.
    - `m_enemyManager->commitAllMoves(m_collisionManager.get())` valida y confirma movimientos.

  6) Visual debug de IA
  - Cada `Enemy` ahora dibuja un cono de visión de 65° orientado según su `facingDir_` (actualizado al moverse). Los colores del cono son configurables con `setVisionColors`.

  Notas de integración
  - Mantener `EntityManager` como propietario único de entidades evita la doble propiedad; `EnemyManager` actúa como orquestador que opera sobre punteros no propietarios.
  - El enfoque facilita pruebas y logging centralizado de bloqueos de movimiento (por ejemplo: logs desde `EnemyManager::commitAllMoves`).

- Observaciones sobre el enfoque:
  - Pre-move validation evita que el `Player` atraviese paredes al bloquear el movimiento antes de aplicarlo.
  - Actualmente se bloquea el movimiento completo si el testBounds solapa; no hay sliding por defecto.
  - `CollisionSystem` sigue presente como mecanismo de corrección y logging (rate-limited) cuando se detectan superposiciones.

Integración con `Game` y assets
- `Game` sigue creando `SceneManager` y haciendo `push(MenuScene)` al arrancar. `CMakeLists.txt` fue actualizado para compilar los nuevos archivos si no estaban incluidos.
- Post-build: los assets se copian a `build/Release/assets` para que las escenas puedan cargar fuentes y texturas.

Quality gates y pruebas rápidas
- Compilación: OK en la rama de trabajo (Release build generada).
- Comportamiento verificado localmente: el `Player` ya no se reposiciona automáticamente en `update()`; la escena controla la aplicación de movimiento (pre-check + commit). La pared se usa para pruebas manuales.

Limitaciones y riesgos
- Bloqueo completo vs sliding:
  - Movimiento actual bloquea todo el desplazamiento si el destino colisiona. Para una experiencia más suave, implementar tests por eje (X/Y) y aplicar ejes no bloqueados.
- Túnel a alta velocidad:
  - El test puntual de bounds puede fallar si la velocidad mueve al jugador más allá de su tamaño en un frame. Soluciones: subdividir movement, swept-AABB o continuous collision detection.
- Estado y eliminación de entidades:
  - `EntityManager` actualiza colliders cada frame; la recomendación de usar mark-for-delete sigue vigente para borrar entidades de forma segura.

Recomendaciones y próximos pasos
- Implementar checks por eje para permitir sliding cuando sólo un eje está bloqueado.
- Añadir swept-AABB o subdivisión de pasos para evitar tunneling a altas velocidades.
- Añadir tests de integración: simular movimiento del `Player` hacia la pared y verificar que la posición final no atraviesa el collider.

Resumen rápido
- `PlayScene` fue extendido para orquestar colisiones: crea y encola `CollisionManager`/`CollisionSystem`, actualiza colliders desde `EntityManager`, y usa un enfoque de pre-move validation seguido de commit/resolve.

Fin del `scene-status`.
