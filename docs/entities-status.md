# Estado actual del módulo `entities`

Resumen corto
- Objetivo: Documentar el estado actual de `src/entities` (Entity, EntityManager, Player), su API pública, integración con escenas y core, cambios recientes y próximos pasos.
- Estado general: Implementación funcional orientada a prototipo. Se añadieron cambios para soportar pre-check de movimiento y coordinación con el módulo de colisiones (feature/Player).

Checklist de lo incluido
- [x] Listado de archivos y responsabilidades.
- [x] API pública y contrato por clase (`Entity`, `EntityManager`, `Player`).
- [x] Integración con `PlayScene`, `InputManager` y el nuevo módulo de colisiones.
- [x] Limitaciones, riesgos y recomendaciones.

Archivos relevantes
- `src/entities/Entity.h` / `src/entities/Entity.cpp` — clase base abstracta.
- `src/entities/EntityManager.h` / `src/entities/EntityManager.cpp` — gestor de entidades (vector de `unique_ptr`). Ahora notifica a `CollisionManager` para añadir/actualizar/remover colliders.
- `src/entities/Player.h` / `src/entities/Player.cpp` — implementación concreta de jugador con nuevo API de movimiento previsto (compute/commit).
 - `src/entities/Player.h` / `src/entities/Player.cpp` — implementación concreta de jugador con nuevo API de movimiento previsto (compute/commit).

Nuevas integraciones relacionadas con IA
- `Enemy` (nuevo) en `src/ai/Enemy.{h,cpp}` es ahora una subclase de `Entity` y se gestiona por `EntityManager` como cualquier otra entidad. Los enemigos usan la misma API de movimiento planificado (`computeIntendedMove` / `commitMove`) para que la escena y el `CollisionManager` puedan validar sus movimientos.
- `EnemyManager` (nuevo) en `src/ai/EnemyManager.{h,cpp}` mantiene punteros no propietarios (`Enemy*`) registrados por `PlayScene` tras crear y añadir las entidades al `EntityManager`. `EnemyManager` centraliza:
  - La ejecución de la FSM de cada enemigo (`updateAll`).
  - La planificación de movimiento collision-aware (`planAllMovement`).
  - El commit de movimientos tras chequeos de colisión (`commitAllMoves`).

Cambios en `PlayScene` y flujo de autoridad
- `PlayScene` ahora crea y mantiene un `EnemyManager`. Al crear enemigos, las entidades se agregan a `EntityManager` (que mantiene ownership via `unique_ptr`) y se registra el puntero en `EnemyManager` mediante `addEnemyPointer`.
- En `PlayScene::update(dt)` el flujo es:
  1. `m_enemyManager->updateAll(dt, playerPos)` — ejecuta FSMs.
  2. `m_enemyManager->planAllMovement(dt, m_collisionManager.get())` — cada enemigo calcula `intendedPosition_` teniendo en cuenta `CollisionManager`.
  3. `m_enemyManager->commitAllMoves(m_collisionManager.get())` — realiza comprobaciones finales y llama `commitMove()` si no hay colisión.

Notas sobre ownership y colisiones
- `EntityManager` sigue siendo la fuente de la verdad para la propiedad de entidades (almacena `std::unique_ptr<Entity>`). `EnemyManager` solo mantiene referencias no propietarias para coordinar IA.
- `EnemyManager::commitAllMoves` reutiliza `CollisionManager::firstColliderForBounds` para validar cada movimiento antes de confirmar.

IA, visión y ataques
- `Enemy` implementa una FSM (IDLE, PATROL, CHASE, ATTACK, RETURN). La detección usa distancia euclídea con `visionRange_`.
- Visual debug: los enemigos dibujan un cono de visión (65°) orientado por `facingDir_` que se actualiza al moverse. Los colores del cono son configurables (`setVisionColors`).
- Ataque básico: `Enemy::attack(Player&)` llama `Player::applyDamage(int)`. Se añadió cooldown para limitar la frecuencia de ataques.

Impacto en `entities` y recomendaciones
- El patrón compute/commit de `Player` se extendió a `Enemy`, por lo que el `Entity` contrato es compatible con la validación centralizada de colisiones.
- Recomendación: conservar `EntityManager` como único propietario y registrar punteros en sistemas que centralizan comportamiento (como `EnemyManager`) para evitar doble propiedad o slicing.


Visión general
El módulo `entities` sigue proveyendo la abstracción mínima para objetos actualizables/repintables. Recientemente se adaptó para integrarse con un sistema de colisiones simple: el `EntityManager` registra bounds en `CollisionManager` y el `Player` expone una API para calcular movimientos sin aplicarlos, de forma que la escena pueda pre-validarlos.

1) Entity (abstracto)
- Ubicación: `src/entities/Entity.h`.
- Propósito: definir atributos y hooks básicos que cualquier entidad debe exponer.
- Atributos principales: `Id id_`, `sf::Vector2f position_`, `sf::Vector2f size_`.
- Métodos públicos:
  - Constructor: `Entity(Id id = 0, position, size)`.
  - id(), position(), setPosition(), size(), setSize().
  - virtual void update(float deltaTime) = 0;
  - virtual void render(sf::RenderWindow& window) = 0;
- Contrato: las subclases deben implementar `update` y `render`. Los getters son noexcept.

2) EntityManager
- Ubicación: `src/entities/EntityManager.h/.cpp`.
- Propósito: contener y administrar la vida de entidades mediante `std::vector<std::unique_ptr<Entity>>`.
- API pública relevante:
  - void addEntity(std::unique_ptr<Entity> entity);
  - bool removeEntity(Entity::Id id);
  - void updateAll(float deltaTime);
  - void renderAll(sf::RenderWindow& window) const;
  - Entity* getEntity(Entity::Id id) const;
  - std::size_t count() const noexcept;
- Nuevas integraciones observadas:
  - `setCollisionManager(CollisionManager*)` permite registrar un `CollisionManager` en el `EntityManager`.
  - Al `addEntity` se registra un collider (bounds) para la entidad si hay un `CollisionManager` asignado.
  - En `updateAll` se actualizan los bounds de cada collider usando la posición/size actuales de cada entidad (evita desincronías).
- Limitaciones actuales:
  - No hay protección de concurrencia (no thread-safe).
  - Búsqueda y eliminación son lineales (O(n)).
  - Sin sistema de orden de renderizado o prioridad.

3) Player (implementación concreta)
- Ubicación: `src/entities/Player.h/.cpp`.
- Propósito: entidad controlable por jugador; maneja entrada, estado, y rendering.
- Atributos principales: `float speed_`, `int health_`, `State state_` (Idle/Walking/Dead), `sf::Vector2f velocity_`, `sf::RectangleShape shape_`, temporizadores para logs.
- Cambios importantes en la API y comportamiento:
  - Nuevo contrato para movimiento: el `Player` ya no aplica su movimiento en `update()` directamente.
    - `sf::Vector2f computeIntendedMove(float deltaTime) const` — calcula la posición objetivo basada en la velocidad actual y `speed_`, sin aplicarla.
    - `void commitMove(const sf::Vector2f& newPosition)` — aplica la posición cuando la escena lo permite.
  - `handleInput(input::InputManager&)` sigue actualizando `velocity_` y `state_`.
  - `update(float)` ahora sincroniza `shape_` con `position_` y sigue emitiendo logs periódicos; la escena es responsable de solicitar/validar el movimiento.
- Observaciones:
  - Esta aproximación permite que la escena (p. ej. `PlayScene`) haga comprobaciones de colisión antes de aplicar el movimiento definitivo, evitando atravesar paredes por reacción post-movimiento.

Integración con escenas y core (flujo actual)
- `PlayScene::onEnter()` crea `EntityManager`, registra un `CollisionManager` y crea `CollisionSystem`.
- Flujo de `PlayScene::update(dt)`:
  1. Procesar input y actualizar el estado/`velocity_` del `Player` (via `handleInput`).
  2. Llamar `EntityManager::updateAll(dt)` para que entidades actualicen su estado y que `EntityManager` actualice bounds en `CollisionManager`.
  3. Calcular la posición objetivo del `Player` con `computeIntendedMove(dt)`.
  4. Construir un `sf::FloatRect` con la posición objetivo y consultar `CollisionManager::firstColliderForBounds(bounds, player)`.
     - Si no hay colisión, llamar `player->commitMove(intended)` para aplicar movimiento.
     - Si hay un bloqueador, registrar el evento (logger) y no aplicar el movimiento.
  5. Invocar `CollisionSystem::resolve(player, dt)` como fallback para corregir solapamientos residuales.
  6. Sincronizar debug visuals con `player->position()`.

Otros cambios prácticos
- Se añadió una entidad `Wall` (static) usada en `PlayScene` para pruebas; en la rama actual la pared se coloca más lejos y un poco más abajo para pruebas (ej. `(480,170)` con tamaño `(50,150)`).
- `CollisionManager` ganó un helper `firstColliderForBounds(const sf::FloatRect&, Entity* exclude)` para tests de pre-movimiento.

Quality gates y estado del build
- El código que implementa estas integraciones compila y el binario se ha generado con éxito en la rama de trabajo (verificación local, Release build OK).

Casos borde y riesgos (actualizados)
- Bloqueo completo vs sliding:
  - Actualmente la escena bloquea el movimiento completo si el `testBounds` colisiona con cualquier collider. Esto evita atravesar paredes pero no permite "sliding" (moverse en el eje no bloqueado). Si quieres movimiento más suave, implementar controles por eje (test X y Y por separado) o swept-AABB.
- Túnel a alta velocidad:
  - El método de comprobación por bounds puntuales puede fallar (tunneling) si el jugador mueve una distancia mayor que su tamaño en un frame. Para entornos con alta velocidad, usar swept-AABB o subdividir el frame.
- Eliminación durante actualización: seguir recomendado marcar para borrado y limpiar fuera del bucle de `updateAll`.

Recomendaciones y próximos pasos
- Si quieres que el jugador patine a lo largo de paredes, implementar checks por eje: probar movimiento en X y Y por separado y aplicar los que no colisionan.
- Considerar swept-AABB para robustez a velocidades altas.
- Añadir tests unitarios para `EntityManager` y tests de integración simples que simulen movimiento del jugador hacia una pared y verifiquen que la posición resultante es correcta.
- Documentar la convención: scene = autoridad para aceptar/rechazar movimientos; `Player` solo calcula y aplica cuando se le indica.

Archivos y responsabilidades (resumen rápido)
- `src/entities/Entity.*` — contrato base (id, position, size, hooks update/render).
- `src/entities/EntityManager.*` — contenedor y gestor de entidades; ahora además actualiza colliders en `CollisionManager`.
- `src/entities/Player.*` — entidad jugable: input, estado, compute/commit movement API, debug logging.

Estado final
- Implementación lista como PoC con integración de colisiones de nivel básico. Comportamiento actual: pre-move blocking (no sliding), residual resolution as fallback, and debug logging.
- Pendientes: tests, opción sliding, swept-AABB for tunnel prevention, safe removal mechanism.

Fin del `entities-status`.
