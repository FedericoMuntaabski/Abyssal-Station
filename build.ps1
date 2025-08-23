<#!
Build helper for Abyssal-Station (Windows)

This script will:
- Initialize the Visual Studio developer environment (VsDevCmd)
- Bootstrap vcpkg (if not present)
- Install SFML via vcpkg (x64)
- Configure and build the project with CMake (Visual Studio 2022, x64)

Run from the repo root: .\build.ps1
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $MyInvocation.MyCommand.Definition
$vcpkgDir = Join-Path $root 'vcpkg'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'

<#
Build helper for Abyssal-Station (Windows)

This script will:
- Initialize the Visual Studio developer environment (VsDevCmd)
- Bootstrap vcpkg (if not present)
- Install SFML via vcpkg (x64)
- Configure and build the project with CMake (Visual Studio 2022, x64)

Run from the repo root: .\build.ps1
#>

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $MyInvocation.MyCommand.Definition
$vcpkgDir = Join-Path $root 'vcpkg'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'

Write-Host "Repo root: $root"

if (-not (Test-Path $vswhere)) {
    Write-Error "vswhere not found at $vswhere. Ensure Visual Studio Installer is installed."
    exit 1
}

$inst = (& $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath).Trim()
if (-not $inst) {
    Write-Error "No Visual Studio installation found via vswhere. Open Visual Studio Installer and ensure 'Desktop development with C++' is installed."
    exit 1
}

Write-Host "Using Visual Studio at: $inst"
Write-Host "Initializing Visual Studio developer environment (x64)..."
& "$inst\Common7\Tools\VsDevCmd.bat" -arch=amd64

Write-Host "Checking compiler (cl.exe):"
where.exe cl.exe

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Error "git not found. Install Git for Windows and re-run this script."
    exit 1
}

if (-not (Test-Path $vcpkgDir)) {
    Write-Host "Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git $vcpkgDir
    Push-Location $vcpkgDir
    .\bootstrap-vcpkg.bat
    Pop-Location
} else {
    Write-Host "vcpkg already present at $vcpkgDir"
}

Write-Host "Installing SFML (x64) via vcpkg..."
Push-Location $vcpkgDir

# Try recommended feature set first (system is not always a vcpkg feature)
Write-Host "Attempting: vcpkg install sfml[graphics,window]:x64-windows"
& .\vcpkg.exe install sfml[graphics,window]:x64-windows
if ($LASTEXITCODE -ne 0) {
    Write-Host "Primary install failed (features may differ). Trying fallback: vcpkg install sfml:x64-windows"
    & .\vcpkg.exe install sfml:x64-windows
    if ($LASTEXITCODE -ne 0) {
        Write-Error "vcpkg failed to install SFML. Check vcpkg output for details."
        Pop-Location
        exit 1
    }
}
Pop-Location

Write-Host "Installing nlohmann-json (x64) via vcpkg..."
Push-Location $vcpkgDir
& .\vcpkg.exe install nlohmann-json:x64-windows
if ($LASTEXITCODE -ne 0) {
    Write-Error "vcpkg failed to install nlohmann-json. Check vcpkg output for details."
    Pop-Location
    exit 1
}
Pop-Location

Write-Host "Configuring and building with CMake (Visual Studio 17 2022, x64)..."
$buildDir = Join-Path $root 'build'
if (-not (Test-Path $buildDir)) { New-Item -ItemType Directory -Path $buildDir | Out-Null }
Push-Location $buildDir

cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$vcpkgDir/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build . --config Release

Pop-Location

$exe = Join-Path $root 'build\Release\AbyssalStation.exe'
if (Test-Path $exe) {
    Write-Host "Build complete. Executable: $exe"
} else {
    Write-Host "Build finished but executable not found at $exe - check CMake output for errors."
}
