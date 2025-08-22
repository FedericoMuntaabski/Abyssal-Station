# Abyssal Station — Build instructions (Windows)

Breve: este repo contiene una clase `Game` (SFML) y un `CMakeLists.txt` mínimo. Si ya instalaste CMake, sigue estos pasos para preparar dependencias (SFML) y compilar en Windows usando `vcpkg`.

Requisitos previos
- `CMake` (ya instalado). Version >= 3.10.
- Un compilador C++ (recomendado: MSVC / Visual Studio Build Tools). Puedes comprobar si tienes `cl.exe` disponible:

```powershell
where.exe cl.exe
```

Si no aparece nada, instala Visual Studio Build Tools (o Visual Studio con el workload "Desktop development with C++").

Instalación recomendada de dependencias (vcpkg + SFML)
1. Clona y bootstrap vcpkg:

```powershell
cd E:\Abyssal-Station
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

2. Instala SFML (tripleta x64 en este ejemplo):

```powershell
.\vcpkg.exe install sfml[graphics,window,system]:x64-windows
```

Compilar el proyecto con CMake (usando vcpkg)
1. Desde la raíz del repo crea el directorio de build y configura CMake pasando el toolchain de vcpkg:

```powershell
cd E:\Abyssal-Station
if (!(Test-Path build)) { New-Item -ItemType Directory build | Out-Null }
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=E:/Abyssal-Station/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
```

2. Compila:

```powershell
cmake --build . --config Release
```

3. Ejecuta (la ruta puede variar según el generador):

```powershell
.\Release\AbyssalStation.exe
```

Alternativa — usar SFML descargado manualmente
- Descarga SFML precompilado desde https://www.sfml-dev.org/download.php (elige la versión que coincida con tu compilador). Descomprime en `C:/libs/SFML` o similar.
- Indica la ruta a CMake:

```powershell
cmake .. -DSFML_DIR="C:/libs/SFML/lib/cmake/SFML"
cmake --build . --config Release
```

Errores comunes
- `'cmake' no se reconoce` — CMake no está en PATH o la terminal necesita reiniciarse.
- `CMake could not find SFML` — apunta `SFML_DIR` o usa `vcpkg` y el `CMAKE_TOOLCHAIN_FILE`.
- `linker errors` por mismatch x86/x64 — asegúrate de instalar SFML para la misma arquitectura que tu compilador (x64 vs x86).

Si quieres, puedo:
- Añadir scripts de PowerShell para automatizar bootstrap + build.
- Ajustar `CMakeLists.txt` para detectar `vcpkg` automáticamente cuando exista.

Fin.
# Abyssal-Station
Abyssal Station es un juego 2D top-down de terror subacuático multijugador. Los jugadores son el staff de un submarino enviado a reparar una estación abandonada, completar objetivos críticos y escapar, mientras evitan a una amenaza hostil controlada por IA en pasillos claustrofóbicos y peligrosos.
