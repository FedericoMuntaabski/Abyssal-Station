# Plan de Implementación - Sistemas de Supervivencia Avanzados
## Abyssal Station - Integración Completa con Escena Actual

---

## 📋 RESUMEN EJECUTIVO

**Objetivo**: Implementar sistemas de supervivencia/horror avanzados en Abyssal Station, integrándolos completamente con la escena actual (`PlayScene`) y manteniendo la arquitectura existente.

**Duración Estimada**: 10 sprints (20 semanas)  
**Prioridad**: Alta  
**Estado Actual**: Planificación Completa  

---

## 🏗️ ARQUITECTURA DE INTEGRACIÓN

### Sistemas Existentes a Extender
```cpp
// PlayScene.h - Managers existentes que se extenderán
std::unique_ptr<entities::EntityManager> m_entityManager;      // ✅ Player extensions
std::unique_ptr<ui::UIManager> m_uiManager;                    // ✅ HUD systems  
std::unique_ptr<gameplay::ItemManager> m_itemManager;          // ✅ Inventory integration
std::unique_ptr<ai::EnemyManager> m_enemyManager;              // ✅ Enhanced AI
std::unique_ptr<collisions::CollisionManager> m_collisionManager; // ✅ Noise/stealth
```

### Nuevos Sistemas a Crear
```cpp
// Nuevos managers para PlayScene
std::unique_ptr<gameplay::InventorySystem> m_inventorySystem;
std::unique_ptr<gameplay::VisionSystem> m_visionSystem;
std::unique_ptr<gameplay::NoiseSystem> m_noiseSystem;
std::unique_ptr<gameplay::CombatSystem> m_combatSystem;
std::unique_ptr<ui::SurvivalHUD> m_survivalHUD;
```

---

## 🗓️ PLAN DE IMPLEMENTACIÓN POR FASES

### 📦 **FASE 1: Estados Vitales y HUD Base** (Sprint 1-2)
**Objetivo**: Implementar recursos vitales del jugador y HUD básico

#### Sprint 1: Extensión del Sistema Player
**Archivos a modificar**:
- `src/entities/Player.h/.cpp`
- `src/entities/PlayerController.h/.cpp`

**Nuevas propiedades en Player**:
```cpp
class Player : public Entity {
private:
    // Estados vitales
    float health_{100.0f};        // Ya existe, extender
    float battery_{100.0f};       // NUEVO
    float fatigue_{0.0f};         // NUEVO  
    float weight_{0.0f};          // NUEVO - calculado dinámicamente
    float maxWeight_{20.0f};      // NUEVO
    
    // Estados de sigilo
    bool isCrouching_{false};     // NUEVO
    bool isMovingSlow_{false};    // NUEVO
    bool flashlightOn_{true};     // NUEVO
    
    // Estados de heridas
    bool hasInjury_{false};       // NUEVO
    float injuryThreshold_{35.0f}; // NUEVO
    int availableSlots_{4};       // NUEVO - reducido por heridas
    
    // Timers
    float batteryRegenTimer_{0.0f};
    float fatigueRecoveryTimer_{0.0f};
    float healthRegenTimer_{0.0f};
};
```

**Métodos a agregar**:
```cpp
// Gestión de estados vitales
void updateVitalSigns(float deltaTime);
void drainFatigue(float amount);
void drainBattery(float amount);
void recoverFatigue(float deltaTime);
void rechargeBattery(float deltaTime);

// Gestión de peso
void calculateWeight();
float getWeightRatio() const;
bool isOverloaded() const;

// Estados de sigilo
void setCrouching(bool crouching);
void setMovingSlow(bool slow);
void toggleFlashlight();

// Sistema de heridas
void checkForCriticalInjury();
void applyInjuryEffects();
```

#### Sprint 2: HUD de Estados Vitales
**Nuevo archivo**: `src/ui/SurvivalHUD.h/.cpp`

**Integración con UIManager**:
```cpp
class SurvivalHUD {
private:
    // Barras de estado (inferior izquierda)
    sf::RectangleShape healthBar_;
    sf::RectangleShape batteryBar_;
    sf::RectangleShape fatigueBar_;
    sf::Text weightText_;
    
    // Barra de estamina dinámica (centro-bajo)
    sf::RectangleShape staminaBar_;
    bool showStaminaBar_{false};
    
    // Iconos de estado
    sf::Sprite flashlightIcon_;
    sf::Sprite stealthIcon_;
    sf::Sprite injuryIcon_;
    
    // Referencias al player
    entities::Player* player_;
    
public:
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    void setPlayer(entities::Player* player);
};
```

**Integración en PlayScene**:
```cpp
// PlayScene.cpp - onEnter()
m_survivalHUD = std::make_unique<ui::SurvivalHUD>();
m_survivalHUD->setPlayer(m_player);

// PlayScene.cpp - update()
m_survivalHUD->update(dt);

// PlayScene.cpp - render() 
m_survivalHUD->render(window);
```

**Criterios de Éxito Fase 1**:
- ✅ Player tiene estados vitales funcionales
- ✅ HUD muestra barras de vida, batería, fatiga y peso
- ✅ Estados se actualizan en tiempo real
- ✅ Integración completa con save/load existente

---

### 🎒 **FASE 2: Sistema de Inventario Restringido** (Sprint 3-4)
**Objetivo**: Implementar inventario con peso y slots limitados

#### Sprint 3: Sistema de Inventario Base
**Nuevo archivo**: `src/gameplay/InventorySystem.h/.cpp`

```cpp
class InventorySystem {
private:
    struct InventorySlot {
        std::shared_ptr<Item> item;
        bool isMainSlot{false};    // true para hotbar
        bool isLocked{false};      // por heridas críticas
    };
    
    std::array<InventorySlot, 4> mainSlots_;     // Hotbar
    std::vector<InventorySlot> backpackSlots_;   // Mochila expandible
    
    float currentWeight_{0.0f};
    float maxWeight_{20.0f};
    
public:
    bool addItem(std::shared_ptr<Item> item);
    bool removeItem(int slotIndex);
    bool swapItems(int fromSlot, int toSlot);
    void calculateTotalWeight();
    
    // Integración con heridas
    void lockSlots(int numSlotsToLock);
    void unlockSlots();
    
    // Serialización
    nlohmann::json saveToJson() const;
    void loadFromJson(const nlohmann::json& json);
};
```

#### Sprint 4: UI del Inventario
**Extensión de SurvivalHUD**:
```cpp
// Hotbar (inferior derecha)
std::array<sf::RectangleShape, 4> hotbarSlots_;
std::array<sf::Sprite, 4> hotbarIcons_;
std::array<sf::Text, 4> hotbarTexts_;
int selectedSlot_{0};

// Mochila completa (overlay)
bool showFullInventory_{false};
sf::RectangleShape backpackBackground_;
std::vector<sf::RectangleShape> backpackSlots_;

// Gestión de entrada
void handleInventoryInput(sf::Event& event);
void selectSlot(int slot);
void toggleFullInventory();
```

**Integración con ItemManager existente**:
```cpp
// ItemManager.cpp - modificar pickup
bool ItemManager::pickupItem(entities::Player* player, ItemId itemId) {
    auto item = getItem(itemId);
    if (!item) return false;
    
    // NUEVO: usar InventorySystem en lugar de inventoryCount_
    if (!m_inventorySystem->addItem(item)) {
        // Mochila llena o sobrepeso
        m_uiManager->showNotification("Inventario lleno", ui::NotificationType::Warning);
        return false;
    }
    
    // Resto de lógica existente...
}
```

**Criterios de Éxito Fase 2**:
- ✅ Inventario funciona con 4 slots principales + mochila
- ✅ Sistema de peso afecta movimiento del jugador
- ✅ Hotbar visual con teclas 1-4 funcional
- ✅ Integración completa con ItemManager existente

---

### 👁️ **FASE 3: Sistema de Visión y Sigilo** (Sprint 5-6)
**Objetivo**: Campo de visión limitado, linterna y mecánicas de sigilo

#### Sprint 5: Sistema de Visión
**Nuevo archivo**: `src/gameplay/VisionSystem.h/.cpp`

```cpp
class VisionSystem {
private:
    // Parámetros de visión
    float visionAngle_{60.0f};          // Grados
    float visionRange_{150.0f};         // Unidades
    float flashlightRange_{200.0f};     // Unidades con linterna
    
    // Renderizado
    sf::VertexArray visionCone_;
    sf::RenderTexture fogOfWar_;
    sf::Shader* lightingShader_;
    
    // Referencias
    entities::Player* player_;
    collisions::CollisionManager* collisionManager_;
    
public:
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    
    // Queries de visión
    bool canSeePoint(const sf::Vector2f& point) const;
    bool isInVisionCone(const sf::Vector2f& point) const;
    bool hasLineOfSight(const sf::Vector2f& from, const sf::Vector2f& to) const;
    
    // Configuración
    void setPlayer(entities::Player* player);
    void setCollisionManager(collisions::CollisionManager* cm);
};
```

#### Sprint 6: Mecánicas de Sigilo
**Extensión de Player**:
```cpp
// Player.cpp - nuevos métodos
void Player::updateStealthState(float deltaTime) {
    // Calcular nivel de detectabilidad
    float detectability = 1.0f;
    
    if (isCrouching_) {
        detectability *= 0.3f;  // 70% menos visible
        speed_ *= 0.5f;         // 50% velocidad
    }
    
    if (isMovingSlow_) {
        detectability *= 0.5f;  // 50% menos detectable
    }
    
    if (!flashlightOn_) {
        detectability *= 0.2f;  // 80% menos visible en oscuridad
    }
    
    currentDetectability_ = detectability;
}
```

**Integración con AI existente**:
```cpp
// Enemy.cpp - modificar detección
bool Enemy::detectPlayer(const sf::Vector2f& playerPos) {
    // Usar detectabilidad del jugador
    float playerDetectability = player_->getDetectability();
    float adjustedRange = visionRange_ * playerDetectability;
    
    // Resto de lógica existente con rango ajustado...
}
```

**Criterios de Éxito Fase 3**:
- ✅ Campo de visión en cono funcional
- ✅ Linterna consume batería y extiende visión
- ✅ Sigilo (agacharse, velocidad) afecta detección
- ✅ Fog of war fuera del campo de visión

---

### 🔊 **FASE 4: Sistema de Ruido y Combate** (Sprint 7-8)
**Objetivo**: Generación de ruido por acciones y combate improvisado

#### Sprint 7: Sistema de Ruido
**Nuevo archivo**: `src/gameplay/NoiseSystem.h/.cpp`

```cpp
class NoiseSystem {
private:
    struct NoiseEvent {
        sf::Vector2f position;
        float intensity;
        float radius;
        float lifetime;
        std::string source;
    };
    
    std::vector<NoiseEvent> activeNoises_;
    
    // Niveles de ruido por acción
    static constexpr float WALK_NOISE = 3.0f;
    static constexpr float RUN_NOISE = 8.0f;
    static constexpr float DOOR_NOISE = 5.0f;
    static constexpr float COMBAT_NOISE = 12.0f;
    
public:
    void generateNoise(const sf::Vector2f& pos, float intensity, const std::string& source);
    void update(float deltaTime);
    
    // Para AI de enemigos
    std::vector<NoiseEvent> getNoisesInRange(const sf::Vector2f& pos, float range) const;
    NoiseEvent* getLoudestNoise(const sf::Vector2f& pos, float range);
    
    // HUD feedback
    float getCurrentNoiseLevel(const sf::Vector2f& playerPos) const;
};
```

**Integración con Player**:
```cpp
// Player.cpp - generar ruido en acciones
void Player::update(float deltaTime) {
    // Lógica existente...
    
    // NUEVO: generar ruido basado en acciones
    if (isMoving()) {
        float noiseLevel = isRunning() ? NoiseSystem::RUN_NOISE : NoiseSystem::WALK_NOISE;
        
        // Reducir ruido si está en sigilo
        if (isCrouching_) noiseLevel *= 0.3f;
        if (isMovingSlow_) noiseLevel *= 0.5f;
        
        m_noiseSystem->generateNoise(getPosition(), noiseLevel, "footsteps");
    }
}
```

#### Sprint 8: Sistema de Combate Improvisado
**Extensión del sistema Item**:
```cpp
// Item.h - nuevas propiedades
class Item {
private:
    // NUEVAS propiedades para armas
    bool isWeapon_{false};
    int damage_{0};
    int maxDurability_{100};
    int currentDurability_{100};
    float attackCooldown_{1.5f};
    
public:
    // Métodos de combate
    int attack();                    // Retorna daño, reduce durabilidad
    bool canAttack() const;
    void repair(int amount);
    bool isBroken() const;
};
```

**Nuevo sistema de combate**:
```cpp
// CombatSystem.h/.cpp
class CombatSystem {
private:
    float lastAttackTime_{0.0f};
    static constexpr float ATTACK_COOLDOWN = 1.5f;
    static constexpr float FATIGUE_PER_ATTACK = 15.0f;
    
public:
    bool attemptAttack(entities::Player* player, std::shared_ptr<Item> weapon);
    void handleCombatInput(entities::Player* player, sf::Event& event);
    
    // Integración con ruido
    void registerNoiseSystem(NoiseSystem* noiseSystem);
};
```

**Criterios de Éxito Fase 4**:
- ✅ Acciones del jugador generan ruido específico
- ✅ Enemigos reaccionan a fuentes de ruido
- ✅ Combate melee con armas improvisadas funcional
- ✅ Durabilidad y cooldown de ataques implementado

---

### 🤖 **FASE 5: IA Avanzada y Heridas Persistentes** (Sprint 9-10)
**Objetivo**: Mejoras finales en IA y sistema de heridas

#### Sprint 9: IA Reactiva Mejorada
**Extensión de AIState**:
```cpp
// AIState.h - nuevos estados
enum class AIState {
    // Estados existentes...
    INVESTIGATE_NOISE,    // NUEVO
    PREDICT_MOVEMENT,     // NUEVO
    STALKING,            // NUEVO
    AMBUSH_PREPARATION   // NUEVO
};
```

**Mejoras en Perception**:
```cpp
// Perception.cpp - detección sensorial múltiple
class Perception {
private:
    // NUEVOS sistemas sensoriales
    float vibrationSensitivity_{50.0f};
    float hearingRange_{200.0f};
    float memoryDuration_{30.0f};
    
    std::vector<sf::Vector2f> rememberedPositions_;
    float lastPlayerSightTime_{0.0f};
    
public:
    // NUEVOS métodos
    bool detectVibrations(const sf::Vector2f& playerPos);
    bool hearNoise(const NoiseSystem& noiseSystem);
    sf::Vector2f predictPlayerMovement() const;
    void updateMemory(float deltaTime);
};
```

#### Sprint 10: Sistema de Heridas Persistentes
**Extensión de Player**:
```cpp
// Player.cpp - sistema de heridas
void Player::checkForCriticalInjury() {
    if (health_ <= injuryThreshold_ && !hasInjury_) {
        hasInjury_ = true;
        
        // Aplicar penalizaciones permanentes
        speed_ *= 0.75f;  // 25% reducción velocidad (cojera)
        availableSlots_ = 2;  // Solo 2 slots disponibles
        
        // Notificar al inventario
        m_inventorySystem->lockSlots(2);
        
        // Feedback visual
        m_uiManager->showNotification("Herida crítica - movilidad reducida", 
                                    ui::NotificationType::Critical);
    }
}

void Player::startSlowRegeneration() {
    if (hasInjury_ && health_ < 50.0f) {
        health_ += 1.0f / 30.0f;  // 1 punto cada 30 segundos
    }
}
```

**Criterios de Éxito Fase 5**:
- ✅ IA enemiga con memoria y predicción
- ✅ Modo acecho psicológico funcional
- ✅ Sistema de heridas con penalizaciones permanentes
- ✅ Regeneración lenta hasta 50% máximo

---

## 🔧 INTEGRACIÓN CON ESCENA ACTUAL

### Modificaciones en PlayScene.h
```cpp
class PlayScene : public Scene {
private:
    // Managers existentes (sin cambios)
    std::unique_ptr<entities::EntityManager> m_entityManager;
    std::unique_ptr<ui::UIManager> m_uiManager;
    std::unique_ptr<ai::EnemyManager> m_enemyManager;
    std::unique_ptr<gameplay::ItemManager> m_itemManager;
    std::unique_ptr<gameplay::PuzzleManager> m_puzzleManager;
    
    // NUEVOS managers para supervivencia
    std::unique_ptr<gameplay::InventorySystem> m_inventorySystem;
    std::unique_ptr<gameplay::VisionSystem> m_visionSystem;
    std::unique_ptr<gameplay::NoiseSystem> m_noiseSystem;
    std::unique_ptr<gameplay::CombatSystem> m_combatSystem;
    std::unique_ptr<ui::SurvivalHUD> m_survivalHUD;
    
    // Auto-save existente (sin cambios)
    core::SaveManager* m_saveManager{nullptr};
    float m_autoSaveTimer{0.0f};
    static constexpr float AUTO_SAVE_INTERVAL{120.0f}; // 2 minutos
};
```

### Inicialización en PlayScene::onEnter()
```cpp
void PlayScene::onEnter() {
    // Inicialización existente...
    
    // NUEVOS sistemas de supervivencia
    m_inventorySystem = std::make_unique<gameplay::InventorySystem>();
    m_visionSystem = std::make_unique<gameplay::VisionSystem>();
    m_noiseSystem = std::make_unique<gameplay::NoiseSystem>();
    m_combatSystem = std::make_unique<gameplay::CombatSystem>();
    
    // Configurar referencias cruzadas
    m_visionSystem->setPlayer(m_player);
    m_visionSystem->setCollisionManager(m_collisionManager.get());
    m_combatSystem->registerNoiseSystem(m_noiseSystem.get());
    
    // HUD de supervivencia
    m_survivalHUD = std::make_unique<ui::SurvivalHUD>();
    m_survivalHUD->setPlayer(m_player);
    m_survivalHUD->setInventorySystem(m_inventorySystem.get());
    
    // Integrar con sistemas existentes
    m_itemManager->setInventorySystem(m_inventorySystem.get());
    m_enemyManager->setNoiseSystem(m_noiseSystem.get());
}
```

### Loop Principal en PlayScene::update()
```cpp
void PlayScene::update(float dt) {
    // Updates existentes...
    m_entityManager->update(dt);
    m_enemyManager->update(dt);
    m_itemManager->update(dt);
    
    // NUEVOS updates de supervivencia
    m_inventorySystem->update(dt);
    m_visionSystem->update(dt);
    m_noiseSystem->update(dt);
    m_combatSystem->update(dt);
    
    // HUD siempre al final
    m_survivalHUD->update(dt);
    
    // Auto-save existente (sin cambios)
    handleAutoSave(dt);
}
```

### Renderizado en PlayScene::render()
```cpp
void PlayScene::render(sf::RenderWindow& window) {
    // Renderizado base existente...
    
    // NUEVO: sistema de visión (fog of war)
    m_visionSystem->render(window);
    
    // Entidades existentes
    m_entityManager->render(window);
    m_itemManager->render(window);
    
    // NUEVO: HUD de supervivencia
    m_survivalHUD->render(window);
    
    // Debug existente (sin cambios)
    if (m_debugOverlay) {
        m_debugOverlay->render(window);
    }
}
```

---

## ⚡ OPTIMIZACIONES DE PERFORMANCE

### Estrategias de Optimización
1. **Sistemas condicionales**: VisionSystem solo actualiza si el jugador se mueve
2. **Pooling de objetos**: Reutilizar NoiseEvent objects
3. **Frustum culling**: Solo procesar enemigos en rango visible
4. **Update scheduling**: Sistemas pesados se actualizan cada N frames
5. **Memory management**: Smart pointers para gestión automática

### Métricas de Performance Objetivo
- **FPS**: Mantener 60 FPS estables
- **Frame time**: Máximo 5ms adicionales por sistema
- **Memory**: Incremento máximo 50MB
- **Loading time**: Sin impacto en transiciones de escena

---

## 🧪 ESTRATEGIA DE TESTING

### Tests por Fase
```cpp
// Ejemplo: tests/survival/VitalStatsTests.cpp
TEST(PlayerVitalStats, BatteryConsumption) {
    Player player;
    player.setBattery(100.0f);
    player.setFlashlightOn(true);
    
    // Simular 1 minuto con linterna
    for (int i = 0; i < 60; ++i) {
        player.update(1.0f);
    }
    
    EXPECT_NEAR(player.getBattery(), 98.0f, 0.1f); // 2 puntos/minuto
}
```

### Tests de Integración
- Interacción entre sistemas (ruido → detección IA)
- Save/load completo de todos los estados
- Performance tests con múltiples sistemas activos
- Memory leak detection

---

## 📊 CRITERIOS DE ÉXITO FINALES

### Funcionales (Debe Cumplir)
- ✅ Todos los estados vitales afectan gameplay
- ✅ Inventario 4+N slots con peso funcional
- ✅ Visión en cono con linterna consumible
- ✅ Sigilo (agacharse, lento) reduce detección
- ✅ Ruido generado atrae enemigos
- ✅ Combate melee con durabilidad
- ✅ Heridas persistentes con penalizaciones
- ✅ IA reactiva con memoria y acecho

### Calidad (Performance)
- ✅ 60 FPS durante gameplay intenso
- ✅ Transiciones de escena < 100ms
- ✅ Save/load completo < 500ms
- ✅ Sin memory leaks detectables

### Experiencia (UX)
- ✅ HUD informativo sin obstrucción
- ✅ Feedback inmediato para todas las acciones
- ✅ Curva de dificultad escalable
- ✅ Tensión balanceada sin frustración

---

## 🚀 ENTREGABLES POR SPRINT

### Sprint 1-2: Base Vital + HUD
- [ ] Player con estados vitales
- [ ] SurvivalHUD básico
- [ ] Integración con save/load
- [ ] Tests unitarios básicos

### Sprint 3-4: Inventario
- [ ] InventorySystem completo
- [ ] Hotbar UI funcional
- [ ] Integración con ItemManager
- [ ] Tests de peso y slots

### Sprint 5-6: Visión + Sigilo
- [ ] VisionSystem con fog of war
- [ ] Mecánicas de sigilo
- [ ] Integración con AI
- [ ] Shader de iluminación

### Sprint 7-8: Ruido + Combate
- [ ] NoiseSystem completo
- [ ] CombatSystem funcional
- [ ] Integración IA-ruido
- [ ] Tests de combate

### Sprint 9-10: IA + Heridas + Polish
- [ ] IA avanzada con memoria
- [ ] Sistema de heridas persistentes
- [ ] Optimizaciones finales
- [ ] Documentación completa

---

**Estado**: 📋 **PLAN COMPLETO - LISTO PARA IMPLEMENTACIÓN**  
**Próximo Paso**: Iniciar Sprint 1 - Extensión del Sistema Player  
**Fecha de Actualización**: 2025-08-25
