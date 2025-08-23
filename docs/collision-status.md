## Estado del módulo `collisions`

Resumen corto

Checklist de requisitos cubiertos

Archivos añadidos / modificados

Resumen técnico y API
  - Constructor: `CollisionBox(Entity* owner, const sf::FloatRect& bounds)`
  - Accesos: `const sf::FloatRect& getBounds() const`, `void setBounds(const sf::FloatRect&)`, `Entity* owner() const`.

  - `void addCollider(Entity* owner, const sf::FloatRect& bounds)` — registra o actualiza el collider de una entidad.
  - `void removeCollider(Entity* owner)` — elimina el collider.
  - `std::vector<Entity*> checkCollisions(Entity* owner) const` — devuelve entidades que colisionan con `owner`.
  - `Entity* firstColliderForBounds(const sf::FloatRect& bounds, Entity* exclude = nullptr) const` — helper para pruebas de pre-movimiento (devuelve el primer collider que intersecta `bounds`).

  - `Entity* firstColliderForBounds(const sf::FloatRect& bounds, Entity* exclude = nullptr, std::uint32_t allowedLayers = 0xFFFFFFFFu) const` — helper para pruebas de pre-movimiento (devuelve el primer collider que intersecta `bounds`). Ahora soporta filtrado por máscara de capas para excluir/permitir interacciones por tipo (ej: items, walls, players).

  - Constructor: `CollisionSystem(CollisionManager& manager)`
  - `void resolve(Entity* entity, float deltaTime)` — resuelve colisiones AABB para `Player` usando MTV y ajusta posición.
  - `void resolveAll()` — placeholder (no implementado: el caller debe iterar entidades si se desea).
  - Logs: rate-limited (por defecto ~0.25s) para no spamear la salida.

  - `CollisionBox` ahora almacena una máscara de capa (`layer_`) tomada del `Entity::collisionLayer()` cuando se registra el collider. Esto permite filtrar colisiones por tipo (Player, Enemy, Item, Wall, etc.).
  - `CollisionManager::segmentIntersectsAny(const sf::Vector2f& p0, const sf::Vector2f& p1, Entity* exclude = nullptr, std::uint32_t allowedLayers = 0xFFFFFFFFu)` — nuevo método que permite comprobar si un segmento cruza algún collider. Implementado con un test de segmento vs AABB (Liang-Barsky style). Útil para línea de visión (LOS) y raycasts simples.

Integración con el resto del proyecto
  - Nuevo método `setCollisionManager(CollisionManager*)` y un puntero `collisionManager_`.
  - En `addEntity` se registra el collider (si el manager está asignado).
  - En `updateAll` se actualizan los bounds de cada collider para que reflejen `position()`/`size()` actuales.

  - Crea `CollisionManager` y `CollisionSystem`, los inyecta en `EntityManager`.
  - Flujo de pre-movimiento: `Player::computeIntendedMove(dt)` → construir `sf::FloatRect` de destino → `firstColliderForBounds(...)` → si no hay colisión `player->commitMove(intended)`; si hay bloqueador, se registra y se evita el commit.
  - Llamada a `CollisionSystem::resolve` como fallback para corregir solapamientos residuales.

  - Nuevos métodos: `computeIntendedMove(float) const` y `commitMove(const sf::Vector2f&)`.
  - `update()` ya no aplica movimiento directamente; la escena es la autoridad que decide aplicar el movimiento.

  - Se añadió la entidad `Wall` para pruebas (`src/entities/Wall.*`).
  - `CMakeLists.txt` actualizado para compilar las nuevas fuentes.

  - `Enemy` (AI) ahora utiliza `CollisionManager::segmentIntersectsAny` para comprobar línea de visión hacia el jugador durante `detectPlayer()`. Esto hace que los muros bloqueen la detección de enemigos incluso si el jugador está dentro del `visionRange_`.

Limitaciones conocidas

Pruebas y estado del build
- Flujo verificado manualmente: el `Player` calcula movimiento, la escena lo valida con `CollisionManager` y lo aplica solo si no hay colisión; `CollisionSystem` corrige solapamientos residuales y emite logs.

- Añadir tests unitarios: `CollisionManager` (add/remove/check), `CollisionSystem` (resolve small scenarios) y casos de integración Player vs Wall.
- Considerar `resolveAll()` o un sistema central que itere entidades relevantes cada frame.
- Módulo `collisions` implementado como PoC y conectado a `entities` y `scene`.
- Comportamiento actual: pre-move blocking + fallback resolution; logging útil para debugging.
