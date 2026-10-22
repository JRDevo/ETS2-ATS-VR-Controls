<#
    install.ps1 -- register ETS2 ATS VR Controls as an implicit OpenXR API layer.

    From the repo (scripts\install.ps1 next to CMakeLists.txt): copies build\Release\{ets2_ats_vr_controls.dll,
    XR_APILAYER_JRDEVO_ets2_ats_vr_controls.json} to the repo root (next to config.ini) first. From an unzipped package
    (install.ps1 next to the DLL): registers the files where they are. -NoCopy: never copy.
    Then writes  HKLM\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit\<full path of the json> = DWORD 0  (0 = enabled).
    HKLM only: ETS2 / ATS use their own built-in OpenXR loader, which reads only HKLM. Never also HKCU: the official loader then
    loads the layer twice. Any per-user copy and any older registration of the same json file name elsewhere are removed.
    Elevates itself (one UAC prompt) unless it already runs as administrator. Refuses while ETS2 / ATS run (the DLL is in use).
      powershell -ExecutionPolicy Bypass -File scripts\install.ps1 [-NoCopy]
#>
param([switch]$NoCopy, [switch]$Elevated)
$ErrorActionPreference = 'Stop'
$jsonName = 'XR_APILAYER_JRDEVO_ets2_ats_vr_controls.json'
$dllName = 'ets2_ats_vr_controls.dll'
$regPath = 'HKLM:\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit'
$repo = (Split-Path $PSScriptRoot -Leaf) -ieq 'scripts' -and (Test-Path (Join-Path (Split-Path -Parent $PSScriptRoot) 'CMakeLists.txt'))
$root = if ($repo) { Split-Path -Parent $PSScriptRoot } else { $PSScriptRoot }

function Fail($m) { Write-Host "ERROR: $m" -ForegroundColor Red; if ($Elevated) { Read-Host 'Press Enter' | Out-Null }; exit 1 }

if (-not $Elevated) {
    # as the user who started it: the game check, the copy, our per-user registration (the elevated process may be another account)
    foreach ($g in 'eurotrucks2', 'amtrucks') { if (Get-Process $g -ErrorAction SilentlyContinue) { Fail "$g.exe is running -- close the game first" } }
    if ($repo -and -not $NoCopy) {
        $rel = Join-Path $root 'build\Release'
        foreach ($f in $dllName, $jsonName) {
            $src = Join-Path $rel $f
            if (-not (Test-Path $src)) { Fail "missing $src (build first)" }
            Copy-Item $src $root -Force
            Write-Host "copied $f -> $root"
        }
    }
    $mine = 'HKCU:\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit'
    if (Test-Path $mine) {
        foreach ($n in ((Get-Item $mine).GetValueNames() | Where-Object { [System.IO.Path]::GetFileName($_) -ieq $jsonName })) {
            Remove-ItemProperty -Path $mine -Name $n
            Write-Host "removed per-user registration: $n"
        }
    }
    $admin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $admin) {
        Write-Host 'Requesting administrator rights (UAC) to write HKLM...'
        $p = Start-Process powershell -Verb RunAs -Wait -PassThru -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"", '-Elevated')
        if ($p.ExitCode -ne 0) { Write-Host "elevated install failed (exit $($p.ExitCode))" -ForegroundColor Red; exit $p.ExitCode }
        & reg query 'HKLM\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit'
        exit 0
    }
}

# ---- HKLM (elevated, or started as administrator)
$json = Join-Path $root $jsonName
if (-not (Test-Path $json)) { Fail "missing $json" }
if (-not (Test-Path (Join-Path $root $dllName))) { Fail "missing $dllName next to $json" }
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
