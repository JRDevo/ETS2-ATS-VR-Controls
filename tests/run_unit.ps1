# run_unit.ps1 -- the unit-test exes (no OpenXR, no game): pad_map_test, profiles_test, config_test (with the repo's config.ini).
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$rel = Join-Path $root 'build\Release'
$bad = 0
foreach ($t in @(@('pad_map_test.exe'), @('profiles_test.exe'), @('config_test.exe', (Join-Path $root 'config.ini')))) {
    Write-Host "=== $($t[0])" -ForegroundColor Cyan
    & (Join-Path $rel $t[0]) @($t | Select-Object -Skip 1)
    if ($LASTEXITCODE -ne 0) { $bad++ }
}
if ($bad) { Write-Host "$bad unit test(s) FAILED" -ForegroundColor Red; exit 1 }
Write-Host 'all unit tests passed' -ForegroundColor Green
