# Run test suite
param(
    [string]$BuildType = "Release",
    [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

$TestExe = Join-Path $BuildDir "$BuildType\vpn_tests.exe"

if (-not (Test-Path $TestExe)) {
    Write-Host "Error: Test executable not found: $TestExe" -ForegroundColor Red
    Write-Host "Please build the project first using scripts\build.ps1" -ForegroundColor Yellow
    exit 1
}

Write-Host "Running test suite..." -ForegroundColor Cyan
& $TestExe

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nAll tests passed!" -ForegroundColor Green
} else {
    Write-Host "`nSome tests failed!" -ForegroundColor Red
    exit 1
}

