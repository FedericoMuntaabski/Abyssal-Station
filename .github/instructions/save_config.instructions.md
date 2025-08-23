---
applyTo: '**/save_config/**'
---

# 🎮 Módulo: Guardado / Configuración

## Objetivo
Implementar un sistema para:
- Guardar/cargar estado del juego (posición jugadores, progreso puzzles, items recolectados).
- Guardar configuraciones de usuario (volumen, resolución, bindings de teclas).
- Utilizar formato de almacenamiento legible (JSON recomendado).
- Mantener independencia del resto de módulos (usar `GameState` como puente).

## Requisitos Técnicos
- Lenguaje: C++  
- Librería sugerida: `nlohmann/json` (JSON para Modern C++).  
- Directorio de persistencia: `saves/` (para partidas), `config/` (para opciones).  
- Integración con:
  - `Core` → para obtener estado del juego.  
  - `UIManager` → para reflejar configuraciones de usuario.  
  - `Logger` → registrar saves/loads exitosos o fallidos.  

## Clases Propuestas
- **ConfigManager**
  - Responsabilidad: leer/escribir archivo `config.json`.
  - Datos: volumen, resolución, bindings de teclas, idioma.
  - Métodos: `loadConfig()`, `saveConfig()`, `applyConfig(UIManager&)`.

- **SaveManager**
  - Responsabilidad: gestionar partidas guardadas.
  - Datos: estado de jugadores, puzzles, items.
  - Métodos: `saveGame(GameState&)`, `loadGame(GameState&)`.

## Reglas de Implementación
- No guardar objetos temporales ni datos de red.  
- Siempre validar si los archivos existen antes de leer.  
- Implementar fallback a configuración por defecto si falla la carga.  
- Versionar esquema de save (campo `"version"`) para compatibilidad futura.  

## Ejemplo de flujo
1. Al iniciar → `ConfigManager.loadConfig()`.  
2. Jugador ajusta opciones en menú → `ConfigManager.saveConfig()`.  
3. Jugador guarda partida → `SaveManager.saveGame(GameState&)`.  
4. Jugador carga partida → `SaveManager.loadGame(GameState&)`.  

## Instrucciones para Copilot
- Generar clases `ConfigManager` y `SaveManager` con métodos declarados.  
- Proponer estructura de JSON para `config.json` y `save.json`.  
- Mostrar ejemplo de guardado y carga desde `PlayScene`.  
- Recordar validar existencia de archivos y fallback a defaults.  
- Incluir logs claros en cada operación (`[save]`, `[load]`, `[config]`).  
