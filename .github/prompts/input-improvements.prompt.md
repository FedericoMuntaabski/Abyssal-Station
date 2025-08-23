---
mode: agent
---

# Objetivo
Mejorar el módulo `InputManager` del proyecto Abyssal Station (C++ / SFML), incluyendo integración con el resto del juego, la UI y escenas. El módulo ya soporta:
- Teclado y mouse
- Multi-bindings por acción
- Detección per-frame correcta (`isActionJustPressed`, `endFrame()`)
- Remapeo en runtime (`OptionsMenu` usa `getBindingName`, `rebindKeys`, `rebindMouse`)

# Requerimientos principales
1. **Serialización de Bindings**  
   Implementar funciones que permitan guardar y cargar configuraciones de controles (teclas y botones) en formato JSON o XML.  
   - Guardar bindings actuales al disco  
   - Cargar bindings al iniciar el juego  
   - Integrar con `OptionsMenu` para que los cambios se reflejen inmediatamente

2. **Vista en Tiempo de Ejecución para Mostrar Bindings Actuales**  
   - Mostrar los bindings actuales en `OptionsMenu`  
   - Permitir remapear acciones y que la UI actualice los labels dinámicamente  
   - Garantizar consistencia con la detección de entradas (`isActionJustPressed`, `isActionPressed`)  

3. **Pruebas Unitarias**  
   - Verificar detección de entradas (teclado y mouse)  
   - Verificar multi-bindings  
   - Verificar serialización y deserialización de configuraciones  
   - Cubrir edge cases: sin bindings, bindings duplicados, inputs inválidos  

# Requerimiento adicional
- Proponer al menos 2 mejoras adicionales que puedan optimizar o expandir la funcionalidad del módulo `InputManager`, como:
  - Mejor manejo de concurrencia  
  - Optimización de consultas de bindings  
  - Registro histórico de inputs  
  - Integración con sistema de tutorial o hints de control  

# Criterios de éxito
- El módulo debe permitir guardar y cargar configuraciones de forma fiable  
- Los bindings deben mostrarse correctamente en tiempo de ejecución y reflejar cambios inmediatamente  
- Las pruebas unitarias deben cubrir todas las funciones nuevas  
- Las mejoras propuestas deben ser relevantes y factibles dentro de la arquitectura actual
