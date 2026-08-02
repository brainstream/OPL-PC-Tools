#Requires -Version 7.0

param(
    [Parameter(Mandatory = $true)]
    [Alias('v')]
    [string]$Version
)

$ErrorActionPreference = 'Stop'

$MSYS2Dir = 'C:\msys64'
$QtVersion = '6.10.0'
$Qt6Dir = "C:\Qt6\$QtVersion\mingw_64"
$BuildDir = Join-Path $PSScriptRoot 'build/windows-amd64'
$ReleaseDir = Join-Path $PSScriptRoot 'release'
$OutputDir = Join-Path $ReleaseDir 'oplpctools'
$ZipPath = Join-Path $ReleaseDir "oplpctools_windows_${Version}_amd64.zip"

Write-Host "============================================================"
Write-Host " OPL PC Tools - Windows Build & Package (amd64)"
Write-Host " Version: $Version"
Write-Host "============================================================"

$env:PATH = "$MSYS2Dir\mingw64\bin;$Qt6Dir\bin;$env:PATH"

if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
    Write-Host "Error: gcc not found. Install mingw-w64-x86_64-toolchain:"
    Write-Host "  pacman -S mingw-w64-x86_64-toolchain"
    exit 1
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "Error: cmake not found. Install cmake:"
    Write-Host "  pacman -S mingw-w64-x86_64-cmake"
    exit 1
}

if (-not (Test-Path "$Qt6Dir\lib\cmake\Qt6")) {
    Write-Host "Error: Qt $QtVersion not found at $Qt6Dir"
    exit 1
}

if (-not (Test-Path "$Qt6Dir\bin\windeployqt.exe")) {
    Write-Host "Error: windeployqt.exe not found in $Qt6Dir\bin"
    exit 1
}

# ============================================================
# 1. Build
# ============================================================

Write-Host ""
Write-Host "[1/3] Building..."
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

Push-Location $BuildDir
try {
    Write-Host "Configuring CMake..."
    & cmake $PSScriptRoot -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$Qt6Dir"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake configuration error"
        exit 1
    }

    Write-Host "Building..."
    & cmake --build . --config Release
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build error"
        exit 1
    }
}
finally {
    Pop-Location
}

# ============================================================
# 2. Package
# ============================================================

Write-Host ""
Write-Host "[2/3] Packaging..."
$ExePath = Join-Path $BuildDir 'oplpctools.exe'
if (-not (Test-Path $ExePath)) {
    Write-Host "Error: $ExePath not found"
    exit 1
}

if (Test-Path -LiteralPath $OutputDir) {
    Remove-Item -LiteralPath $OutputDir -Recurse -Force
}
New-Item -ItemType Directory -Path $OutputDir | Out-Null

$DestExe = Join-Path $OutputDir 'oplpctools.exe'

Write-Host "Copying exe..."
Copy-Item -LiteralPath $ExePath -Destination $DestExe -Force

Write-Host "Running windeployqt..."
& windeployqt --release --qmldir (Join-Path $PSScriptRoot 'src/OplPcTools') $DestExe
if ($LASTEXITCODE -ne 0) {
    Write-Host "windeployqt error"
    exit 1
}

Write-Host "Copying MinGW runtime DLLs..."
foreach ($dll in @('libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll')) {
    Copy-Item -LiteralPath (Join-Path "$MSYS2Dir\mingw64\bin" $dll) -Destination (Join-Path $OutputDir $dll) -ErrorAction SilentlyContinue
}

# ============================================================
# 3. Create ZIP archive
# ============================================================

Write-Host ""
Write-Host "[3/3] Creating ZIP archive..."
Compress-Archive -Path $OutputDir -DestinationPath $ZipPath -Force
Remove-Item -LiteralPath $OutputDir -Recurse -Force

Write-Host ""
Write-Host "============================================================"
Write-Host " Done! Build and packaging completed successfully."
Write-Host "============================================================"
Write-Host ""
Write-Host "ZIP archive: $ZipPath"
