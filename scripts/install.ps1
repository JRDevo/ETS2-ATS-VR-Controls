<#
    install.ps1 -- register VR Controller Keys as an implicit OpenXR API layer.

    Copies build\Release\{controller_keys.dll, XR_APILAYER_JRDEVO_controller_keys.json} next to config.ini (the repo root), then
    writes  HKLM\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit\<full path of the json> = DWORD 0  (0 = enabled).
    HKLM only: ETS2 / ATS use their own built-in OpenXR loader, which reads only HKLM. Never also HKCU: the official loader then
    loads the layer twice. Any per-user copy and any older registration of the same json file name elsewhere are removed.
    Elevates itself (one UAC prompt). Run it from the repo, or from an unzipped package (-NoCopy registers the files where they are).
    Do not run it while a game is running (the DLL is in use).
      powershell -ExecutionPolicy Bypass -File scripts\install.ps1 [-NoCopy]
#>
param([switch]$NoCopy, [switch]$Elevated)
$ErrorActionPreference = 'Stop'
$jsonName = 'XR_APILAYER_JRDEVO_controller_keys.json'
$regPath = 'HKLM:\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit'
$root = Split-Path -Parent $PSScriptRoot
if (-not (Test-Path (Join-Path $root 'config.ini'))) { $root = $PSScriptRoot }   # unzipped package: scripts next to the files

function Fail($m) { Write-Host "ERROR: $m" -ForegroundColor Red; if ($Elevated) { Read-Host 'Press Enter' | Out-Null }; exit 1 }

$admin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $admin) {
    foreach ($g in 'eurotrucks2', 'amtrucks') { if (Get-Process $g -ErrorAction SilentlyContinue) { Fail "$g.exe is running -- close the game first" } }
    if (-not $NoCopy) {
        $rel = Join-Path $root 'build\Release'
        foreach ($f in 'controller_keys.dll', $jsonName) {
            $src = Join-Path $rel $f
            if (-not (Test-Path $src)) { Fail "missing $src (build first)" }
            Copy-Item $src $root -Force
            Write-Host "copied $f -> $root"
        }
    }
    # our per-user registration (if any) is removed here, as the user: the elevated process may run as another account
    $mine = 'HKCU:\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit'
    if (Test-Path $mine) {
        foreach ($n in ((Get-Item $mine).GetValueNames() | Where-Object { [System.IO.Path]::GetFileName($_) -ieq $jsonName })) {
            Remove-ItemProperty -Path $mine -Name $n
            Write-Host "removed per-user registration: $n"
        }
    }
    Write-Host 'Requesting administrator rights (UAC) to write HKLM...'
    $p = Start-Process powershell -Verb RunAs -Wait -PassThru -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"", '-Elevated', '-NoCopy')
    if ($p.ExitCode -ne 0) { Write-Host "elevated install failed (exit $($p.ExitCode))" -ForegroundColor Red; exit $p.ExitCode }
    & reg query 'HKLM\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit'
    exit 0
}

# ---- elevated part: HKLM
$json = Join-Path $root $jsonName
if (-not (Test-Path $json)) { Fail "missing $json" }
if (-not (Test-Path (Join-Path $root 'controller_keys.dll'))) { Fail "missing controller_keys.dll next to $json" }
if (-not (Test-Path $regPath)) { New-Item -Path $regPath -Force | Out-Null }
foreach ($n in (Get-Item $regPath).GetValueNames()) {
    if ([System.IO.Path]::GetFileName($n) -ieq $jsonName -and $n -ne $json) {
        Remove-ItemProperty -Path $regPath -Name $n
        Write-Host "removed older registration: $n"
    }
}
New-ItemProperty -Path $regPath -Name $json -PropertyType DWord -Value 0 -Force | Out-Null
Write-Host "registered: $json" -ForegroundColor Green
if ($Elevated) { Start-Sleep -Seconds 2 }
exit 0
