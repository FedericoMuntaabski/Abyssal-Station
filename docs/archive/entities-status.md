# Estado actual del módulo `entities`

Resumen corto
- Objetivo: Documentar el estado actual de `src/entities` (Entity, EntityManager, Player) y las modificaciones recientes que facilitan la serialización del estado, integración con saves/config y la interoperabilidad con otros subsistemas (colisiones, IA, UI).
- Estado general: Implementación funcional orientada a prototipo; se añadieron accesores para integración (`allEntities()`), se mantuvo el patrón compute/commit y se acomodó la serialización via `GameState`.

Checklist de lo incluido
- [x] Listado de archivos y responsabilidades.
- [x] API pública y contrato por clase (`Entity`, `EntityManager`, `Player`).
- [x] Integración con `PlayScene`, `InputManager`, `CollisionManager`, y los nuevos módulos `ConfigManager`/`SaveManager`.
- [x] Limitaciones, riesgos y recomendaciones.

Archivos relevantes
- `src/entities/Entity.h` / `src/entities/Entity.cpp` — clase base abstracta (id, position, size, collision layer).
- `src/entities/EntityManager.h` / `src/entities/EntityManager.cpp` — gestor de entidades (vector de `unique_ptr`). Ahora además expone `allEntities()` para integración/serialización y sincroniza colliders en `CollisionManager`.
- `src/entities/Player.h` / `src/entities/Player.cpp` — entidad jugable con API compute/commit, manejo de input y helpers (`onItemCollected`, contador de inventario).

Cambios principales y nuevas integraciones

1) Accesores públicos para integración
- Se añadieron métodos no propietarios para facilitar serialización e integración:
  - `EntityManager::allEntities()` → `std::vector<Entity*>`.
  - `ItemManager::allItems()` → `std::vector<Item*>` (en `src/gameplay`).
  - `PuzzleManager::allPuzzles()` → `std::vector<Puzzle*>` (en `src/gameplay`).
- Estos accesores se usan en `scene::SaveIntegration` para construir y aplicar `core::GameState` (guardar/restaurar partidas).

2) Guardado / restauración (SaveIntegration)
- `scene::gatherGameState` extrae el estado actual (players positions/health, itemsCollected, puzzlesCompleted) usando los accesores anteriores.
- `scene::applyGameState` restaura posiciones y flags de items/puzzles; cuando no existen setters directos utiliza llamadas públicas (por ejemplo `Item::interact`) para disparar la lógica de recogida y notificaciones. Recomendado: exponer setters directos para restauración precisa.

3) Player: compute/commit y coleccionables
- `Player` mantiene la API `computeIntendedMove(float)` y `commitMove(const sf::Vector2f&)` — la escena es la autoridad que valida colisiones antes de `commitMove`.
- Se añadió `onItemCollected(Entity::Id)` y `inventoryCount_` para seguimiento simple de items recogidos; `ItemManager` llama a `player->onItemCollected()` cuando un item es recogido por colisión o interacción.

4) Integración con IA y EnemyManager
- `Enemy` sigue el mismo patrón compute/commit y se gestiona por `EntityManager` con referencias no propietarias en `EnemyManager`.

5) Logs y tolerancia a fallos
- `EntityManager::addEntity`/`removeEntity` y `Player`/`Enemy` emiten logs via `core::Logger` para creación/destrucción y eventos importantes.
- Las operaciones de guardado/carga usan `SaveManager` y `ConfigManager` con mensajes claros en fallos y fallbacks seguros (p. ej. `[save] Archivo no encontrado: <ruta>`, `[config] Usando valores por defecto; no se encontró: <ruta>`).

Visión general (flujo de movimiento y colisiones)
- Flujo en `PlayScene::update(dt)` (resumen):
  1. `Player::handleInput()` actualiza `velocity_`.
  2. `EntityManager::updateAll(dt)` actualiza entidades y sincroniza colliders.
  3. `computeIntendedMove` se usa para obtener la posición objetivo.
  4. `CollisionManager::firstColliderForBounds` revisa colisiones; si está libre → `commitMove`, sino se registra bloqueo y no se aplica el movimiento.
  5. `CollisionSystem::resolve` actúa como fallback para solapamientos residuales.

Limitaciones y riesgos (actualizados)
- Bloqueo completo vs sliding: la implementación actual bloquea por completo movimientos con colisión detectada; para sliding o movimiento más natural, probar checks por eje o swept-AABB.
- Túnel a alta velocidad: el test puntual de bounds puede fallar si la distancia de movimiento es mayor que el tamaño del collider; considerar swept tests o subdividir el movimiento.
- Restauración exacta: sin setters públicos para salud/items/puzzles la restauración hace "best effort"; exponer setters facilitará pruebas deterministas.

Recomendaciones y próximos pasos
- Exponer setters públicos para restauración exacta: `Player::setHealth(int)`, `Item::setCollected(bool)`, `Puzzle::forceComplete()`.
- Implementar checks por eje o swept-AABB para mejorar la robustez del movimiento y evitar tunneling.
- Añadir tests unitarios: `EntityManager` (add/remove/update), integración movement vs wall, round-trip `GameState` serialization.

Resumen final
- El módulo `entities` conserva su diseño original (contrato simple Entity/EntityManager) y ha recibido pequeños cambios orientados a integración: accesores para serialización, eventos de recogida de items, y compatibilidad con el patrón compute/commit que facilita la validación centralizada de colisiones. El código compila y está integrado con el resto del sistema (scene, input, save/config).

Fin del `entities-status`.
