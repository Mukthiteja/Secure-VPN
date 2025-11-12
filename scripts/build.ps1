# Build script for CustomVPN on Windows
param(
    [string]$BuildType = "Release",
    [string]$VcpkgRoot = $env:VCPKG_ROOT,
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

if (-not $VcpkgRoot) {
    Write-Host "Error: VCPKG_ROOT environment variable not set." -ForegroundColor Red
    Write-Host "Please set it to your vcpkg installation path, e.g.:" -ForegroundColor Yellow
    Write-Host "  `$env:VCPKG_ROOT = 'D:\vcpkg'" -ForegroundColor Yellow
    exit 1
}

$ToolchainFile = Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"
if (-not (Test-Path $ToolchainFile)) {
    Write-Host "Error: vcpkg toolchain file not found: $ToolchainFile" -ForegroundColor Red
    exit 1
}

Write-Host "Configuring CMake..." -ForegroundColor Cyan
cmake -B $BuildDir -S . `
    -DCMAKE_TOOLCHAIN_FILE="$ToolchainFile" `
    -DVCPKG_TARGET_TRIPLET=x64-windows `
    -DCMAKE_BUILD_TYPE=$BuildType `
    -DCUSTOMVPN_BUILD_TESTS=ON

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    exit 1
}

Write-Host "Building project..." -ForegroundColor Cyan
cmake --build $BuildDir --config $BuildType --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host "`nBuild completed successfully!" -ForegroundColor Green
Write-Host "Executable location: $BuildDir\$BuildType\customvpn.exe" -ForegroundColor Green

