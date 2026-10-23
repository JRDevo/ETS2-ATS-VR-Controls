# run_unit.ps1 -- the unit-test exes (no OpenXR, no game): pad_map_test, profiles_test, config_test (with the shipped config.ini),
# vr_menu_test (writes build\menu_test: a config.ini copy + the panel pictures as .bmp).
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$rel = Join-Path $root 'build\Release'
$bad = 0
# config_test checks the SHIPPED config.ini (as committed or staged): the installed one next to the DLL may hold the VR menu's saved changes
$shipped = Join-Path $root 'build\config_shipped.ini'
$text = git -C $root show :config.ini 2>$null
if ($LASTEXITCODE -eq 0 -and $text) { $text -join "`n" | Set-Content -Encoding ASCII $shipped } else { Copy-Item (Join-Path $root 'config.ini') $shipped }
foreach ($t in @(@('pad_map_test.exe'), @('profiles_test.exe'), @('config_test.exe', $shipped),
                 @('vr_menu_test.exe', (Join-Path $root 'build\menu_test')))) {
    Write-Host "=== $($t[0])" -ForegroundColor Cyan
    & (Join-Path $rel $t[0]) @($t | Select-Object -Skip 1)
    if ($LASTEXITCODE -ne 0) { $bad++ }
}
if ($bad) { Write-Host "$bad unit test(s) FAILED" -ForegroundColor Red; exit 1 }
Write-Host 'all unit tests passed' -ForegroundColor Green
