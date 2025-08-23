# Estado actual del módulo `entities`

Resumen corto
- Objetivo: Documentar el estado actual de `src/entities` (Entity, EntityManager, Player), su API pública, integración con escenas y core, limitaciones y próximos pasos.
- Estado general: Implementación sencilla y funcional orientada a un prototipo. `Entity` es la interfaz base, `EntityManager` maneja una colección de entidades (propiedad exclusiva) y `Player` es una entidad concreta con entrada, movimiento y logging de estado.

Checklist de lo incluido
- [x] Listado de archivos y responsabilidades.
- [x] API pública y contrato por clase (`Entity`, `EntityManager`, `Player`).
- [x] Integración con `PlayScene` y `InputManager`.
- [x] Limitaciones, riesgos y recomendaciones.

Archivos relevantes
- `src/entities/Entity.h` / `src/entities/Entity.cpp` — clase base abstracta.
- `src/entities/EntityManager.h` / `src/entities/EntityManager.cpp` — gestor de entidades (vector de `unique_ptr`).
- `src/entities/Player.h` / `src/entities/Player.cpp` — implementación concreta de jugador.

Visión general
El módulo `entities` proporciona una abstracción mínima para objetos actualizables/repintables en pantalla. Está diseñado como base para tener actores controlables (Player) y futuros NPCs.

1) Entity (abstracto)
- Ubicación: `src/entities/Entity.h`.
- Propósito: definir los atributos y hooks básicos que cualquier entidad debe exponer.
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
- Comportamiento observado:
  - `addEntity` registra la adición usando `core::Logger` y toma propiedad de la entidad.
  - `removeEntity` busca por id y borra la entidad, también registrando la operación.
  - `updateAll` y `renderAll` iteran sobre el vector y llaman a los métodos correspondientes de cada entidad.
- Limitaciones actuales:
  - No hay protección de concurrencia (no thread-safe).
  - Búsqueda y eliminación son lineales (O(n)). Para muchas entidades considerar otra estructura.
  - Sin sistema de orden de renderizado o prioridad; el orden es el orden de inserción.

3) Player (implementación concreta)
- Ubicación: `src/entities/Player.h/.cpp`.
- Propósito: ejemplo de entidad controlable por jugador; maneja entrada, movimiento y rendering.
- Atributos principales: `float speed_`, `int health_`, `State state_` (Idle/Walking/Dead), `sf::Vector2f velocity_`, `sf::RectangleShape shape_`, y temporizador para logs debug.
- API y comportamiento:
  - Constructor: `Player(Id id, position, size, speed, health)` inicializa forma, posición y registra creación en `Logger`.
  - void handleInput(input::InputManager& inputManager): consulta acciones mapeadas (MoveLeft/Right/Up/Down) y actualiza `velocity_` y `state_`. Cambios de estado son loggeados.
  - void update(float deltaTime): normaliza la velocidad, actualiza `position_` y la `shape_`, y emite logs debug periódicos con posición/velocidad.
  - void render(sf::RenderWindow& window): dibuja la `shape_`.
- Observaciones:
  - `Player` depende de `input::InputManager` y de `core::Logger`.
  - No contiene lógica de colisiones ni límites de ventana; se asume que la escena/entorno las maneja.

Integración con escenas y core
- `PlayScene::onEnter()` crea una instancia de `entities::EntityManager` y añade un `entities::Player` (ejemplo: id 1) al manager. `PlayScene::update()` sincroniza el rectángulo de debug con la posición del `Player` y llama a `m_entityManager->updateAll(dt)` y `renderAll(window)` desde `render()`.
- `Player::handleInput` es invocado desde la escena (PlayScene) usando `InputManager::getInstance()`.

Quality gates y estado del build
- Código fuente relacionado compila junto con el resto del proyecto (se verificó que `PlayScene` utiliza `EntityManager` y `Player`).
- No existen tests unitarios para `EntityManager` ni para `Player` en el repositorio actual.

Casos borde y riesgos
- Entidades vacías: `EntityManager` itera sin protección sobre punteros nulos comprobando `if (e)`; sin embargo, la propiedad exclusiva evita mayoría de casos inválidos.
- Eliminación durante actualización: si una entidad se elimina desde su propio `update()`, la implementación actual (iteración simple) podría producir comportamiento indefinido si no se gestiona cuidadosamente. Recomendado usar marca de borrado y limpieza fuera del bucle de actualización.
- Escalabilidad: para grandes cantidades de entidades, la estructura actual podría ser ineficiente en búsqueda/eliminación y en orden de render.

Recomendaciones y próximos pasos
- Añadir tests unitarios para `EntityManager` (add/remove/get/count + update/render doble) y para `Player` (handleInput -> state, update -> position).
- Implementar un mecanismo de eliminación segura (mark-for-delete) para evitar borrar elementos del vector mientras se itera.
- Considerar separar la representación gráfica de la lógica (componentizar shape/position) o migrar a un ECS si el proyecto escala.
- Añadir colisiones básicas y límites de ventana en `Player` o en un sistema de `Physics` separado.
- Documentar claramente ownership/vida de las entidades (por ejemplo en `EntityManager.h`) y posibles invariantes.

Archivos y responsabilidades (resumen rápido)
- `src/entities/Entity.*` — contrato base (id, position, size, hooks update/render).
- `src/entities/EntityManager.*` — contenedor y gestor de entidades (add/remove/updateAll/renderAll).
- `src/entities/Player.*` — entidad jugable: entrada, movimiento, debug logging.

Estado final
- Implementación lista como PoC/primer paso. Integrado en `PlayScene`. Pendientes: tests, eliminación segura, centralización de sistemas (colisiones/physics) y mejoras de performance si se requiere escalar.

Fin del `entities-status`.
