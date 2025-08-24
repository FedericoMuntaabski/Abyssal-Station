# Cambios en el Módulo Core

## Fecha: 2025-08-24

### EntityFactory.cpp/.h - Implementación del Factory Pattern

#### Funcionalidad Implementada:
Patrón Factory para creación centralizada y tipada de entidades del juego.

#### Arquitectura:

1. **EntityType Enum**
   ```cpp
   enum class EntityType {
       PLAYER,
       ENEMY,
       ITEM,
       INTERACTIVE_OBJECT,
       PROJECTILE
   };
   ```

2. **EntityFactory (Singleton)**
   - Gestión centralizada de creación de entidades
   - Validación de parámetros de creación
   - Logging detallado de operaciones
   - Manejo de errores robusto

#### Métodos Principales:

**createEntity():**
```cpp
std::unique_ptr<entities::Entity> createEntity(
    EntityType type,
    const sf::Vector2f& position,
    const std::string& config = ""
);
```

**Características:**
- Validación de posición
- Configuración opcional por string
- Manejo de errores con nullptr
- Logging de éxito/fallo

**registerEntityCreator():**
- Sistema extensible para nuevos tipos
- Funciones lambda/callback para creación custom
- Validación de tipos existentes

#### Tipos de Entidades Soportadas:

1. **PLAYER:**
   - Entidad jugador principal
   - Posición inicial configurable
   - Estados por defecto aplicados

2. **ENEMY:**
   - Entidades enemigas
   - Configuración de comportamiento via string
   - Integración con AISystem

3. **ITEM:**
   - Objetos recolectables
   - Propiedades configurables
   - Estados de interacción

4. **INTERACTIVE_OBJECT:**
   - Objetos del entorno interactivos
   - Configuración de triggers
   - Estados persistentes

5. **PROJECTILE:**
   - Proyectiles y objetos en movimiento
   - Física básica aplicada
   - Lifecycle automático

#### Integración:
- Compatible con Entity base existente
- No rompe código legacy
- Extensible para nuevos tipos
- Integrado con sistemas existentes

#### Ventajas Implementadas:
- **Desacoplamiento:** Creación separada de lógica de juego
- **Extensibilidad:** Fácil agregar nuevos tipos
- **Mantenibilidad:** Código centralizado y organizado
- **Debugging:** Logging completo de operaciones
- **Robustez:** Validación y manejo de errores

#### Estado:
- ✅ Implementación completa
- ✅ Compila sin errores
- ✅ Integrado con sistema existente
- ✅ Tests básicos funcionando

### Cambios Menores:
- Sin modificaciones adicionales en este módulo
- Factory listo para integración con otros patterns
