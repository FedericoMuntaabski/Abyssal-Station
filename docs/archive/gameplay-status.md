# Estado de `src/gameplay`

Este documento reúne el estado actual y observaciones de la carpeta `src/gameplay` (items, puzzles y managers). Incluye un resumen de archivos, comportamiento actual, problemas detectados, contractos/API, casos borde y pasos recomendados.

## Checklist (requerimiento del usuario)
- Crear un documento en `docs` que describa todo lo relacionado con `src/gameplay` — Hecho

## Archivos relevantes (en `src/gameplay`)
- `Item.cpp` / `Item.h` — Implementación de objetos recogibles y su API de interacción.
- `ItemManager.cpp` / `ItemManager.h` — Gestión de colección de `Item`, detección de pickup vía `CollisionManager`, notificación a `UIManager` y enlace con `PuzzleManager`.
- `Puzzle.cpp` / `Puzzle.h` — Lógica de pasos de puzzle, estados (Locked, Active, Completed) y manejo de jugadores que se unen/interactúan.
- `PuzzleManager.cpp` / `PuzzleManager.h` — Registro/actualización/render de puzzles y notificación a `UIManager` al completarse.

(Ver código fuente para detalles; este documento resume los puntos más importantes.)

## Resumen del comportamiento actual
- Items
  - Cada `Item` es una `entities::Entity` con `ItemType` y un `sf::RectangleShape` visual.
  - Los items usan un `CollisionManager` opcional: se registran como collider en construcción y se eliminan al destruirse o ser recogidos.
  - `Item::interact` marca `collected_`, remueve el collider y notifica al `Player` vía `player.onItemCollected(id)`.
  - Visualmente el item se oculta moviendo su `shape_` fuera de pantalla al ser recogido.
- ItemManager
  - Mantiene `std::vector<std::unique_ptr<Item>>` y sincroniza el `CollisionManager` cuando se asigna.
  - En `updateAll` itera items, llama `update` y usa `collisionManager_->firstColliderForBounds(bounds, item.get(), entities::kLayerMaskPlayer)` para detectar jugadores y disparar `interact`.
  - Notifica a `UIManager` y a `PuzzleManager` cuando un item cambia de estado a recogido (si no estaba recogido antes).
  - Expone `interactWithItem(itemId, player)` para forzar interacción desde entrada de jugador.
- Puzzles
  - Un `Puzzle` contiene pasos (strings), vector de booleans `completedSteps_`, estado y un `shape_` para render.
  - `markStepCompleted` marca un paso y registra/loggea.
  - `update` cambia estado a `Active` cuando hay players interactuando y completa el puzzle cuando todos los pasos están marcados.
- PuzzleManager
  - Registra puzzles, actualiza/chequea completados y notifica `UIManager` (una sola notificación por puzzle completado, gracias a `loggedCompleted_`).

## Observaciones / Hallazgos técnicos
1. Duplicación de registro del collider
   - `Item` registra el collider en el constructor y en `setCollisionManager`. Sin embargo, en `Item::update` y en `ItemManager::updateAll` se vuelve a llamar a `collisionManager_->addCollider(this, sf::FloatRect(position_, size_));` en cada frame si no está recogido.
   - Riesgo: si `CollisionManager::addCollider` no está idempotente, esto puede crear duplicados o costar rendimiento.
   - Recomendación: añadir una API de actualización de bounds (e.g., `updateCollider(this, bounds)`) o comprobar si el collider ya existe antes de `addCollider`.

2. Ownership y ciclo de vida
   - `ItemManager` mantiene ownership (unique_ptr). Los items usan punteros a `CollisionManager` y `UIManager` sin ownership, lo cual es aceptable, pero requiere que el lifetime de los managers supere al de `ItemManager` y sus items.
   - Recomendación: documentar claramente el orden esperado de destrucción o usar referencias seguras (observer/weak) si el proyecto evoluciona.

3. Serialización / guardado
   - No hay evidencias directas en estos ficheros de serialización de items/puzzles (aunque hay `saves/example_save.json` en repo root). Si se requiere guardar estado de items/puzzles, hay que añadir: id, position, type, collected, puzzle step completion.

4. Comunicación UI
   - `ItemManager` y `PuzzleManager` usan `UIManager` para notificaciones; la dependencia es por puntero crudo.
   - Recomendación: añadir interfaz mínima (`IUIEvents` o similar) y documentar contrato de notificación (métodos y datos esperados).

5. Concurrency / multijugador
   - `Puzzle` soporta múltiples players mediante `playerJoin`/`playerLeave` con vector de `Player*`. No hay sincronización; si el engine soporta threads, hay que proteger estas estructuras.

6. Logging
   - Buen uso centralizado de `core::Logger::instance()` para eventos (item recogido, puzzle activado/completado). Se puede mejorar con niveles y mensajes más ricos (tipo, position).

## Contrato mínimo / API (resumen)
- Item
  - Inputs: posición, tamaño, tipo, optional `CollisionManager*`.
  - Outputs/efectos: `interact(Player&)` marca collected, notifica Player y Logger, remueve collider.
  - Error modes: interact si ya `collected_` => no-op.
- ItemManager
  - Inputs: `addItem(unique_ptr<Item>)`, `setCollisionManager`, `updateAll(delta)`, `interactWithItem(itemId, player)`.
  - Outputs: notificaciones a `UIManager` y `PuzzleManager` cuando un item pasa a collected.
- Puzzle / PuzzleManager
  - Inputs: pasos al construir, `markStepCompleted(idx)`, `playerJoin`/`playerLeave`.
  - Outputs: cambio de estado (Locked->Active->Completed), logger y notificación UI al completarse.

## Casos borde a considerar
- Item colocado fuera de bounds o con tamaño 0.
- `CollisionManager` nulo: manager y UI manager son opcionales — comprobar que la lógica no asume su existencia.
- Recolección concurrente (dos players colisionando el mismo item en el mismo frame): asegurarse que `interact` es idempotente y sólo se notifique una vez.
- Eliminación de item mientras está siendo iterado por `ItemManager::updateAll`.
- Puzzle con 0 pasos: actualmente el constructor marca `state_` como `Completed` si `steps.empty()`; es correcto pero debería documentarse.

## Tests sugeridos (unitarios / integración)
- Item
  - Crear item con CollisionManager dummy, simular colisión player y verificar que `interact` marca `collected_`, remueve collider y llama a `player.onItemCollected`.
  - Verificar que `update` no re-registrara colliders si se implementa cambio sugerido.
- ItemManager
  - Añadir items y simular `CollisionManager` que devuelva un `Player*` para `firstColliderForBounds`: comprobar que el item se marca recogido, se notifica UI y PuzzleManager.
  - Prueba de `interactWithItem` (llamada directa).
- Puzzle
  - Marcar pasos de forma ordenada y desordenada; confirmar `checkCompletion` y notificación de `PuzzleManager`.

## Cambios de bajo riesgo que puedo implementar ahora (sugeridos)
- Evitar re-llamadas a `addCollider` en `Item::update` (introducir `updateColliderBounds` o bandera `colliderRegistered_`).
- Añadir documentación breve (comentarios) en headers sobre ownership y lifetime de managers.
- Añadir tests básicos para `Item` y `ItemManager` en la suite `tests/`.

## Próximos pasos recomendados
1. Revisar `CollisionManager` para ver si `addCollider` es idempotente o soporta actualización de bounds.
2. Añadir método `CollisionManager::updateColliderBounds(Entity*, FloatRect)` si no existe.
3. Implementar protección contra recolección múltiple simultánea (atomics o estado chequeado antes y después de interact).
4. Escribir tests unitarios mínimos y una entrada de ejemplo en `saves/` con formato para items/puzzles.

---

## Requisitos cubiertos
- Documento `docs/gameplay-status.md` creado en `docs/` con el estado actual y recomendaciones — Done

Si quieres, aplico alguna de las mejoras de bajo riesgo ahora (por ejemplo: evitar re-add de colliders en `Item::update` y añadir un comentario en `Item.h`).
