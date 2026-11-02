<#
    build_setup.ps1 -- build the Windows setup (installer\ets2_ats_vr_controls.iss, Inno Setup 6) from build\Release.
    Output: build\setup\ETS2-ATS-VR-Controls-Setup-v<version>.exe (version = project() in CMakeLists.txt).
    The setup ships the config.ini AS COMMITTED (git show HEAD:config.ini): the root one may hold the VR menu's saved changes.
    ISCC.exe: -Iscc <path>, else PATH, else the usual Inno Setup 6 folders. Never runs the setup.
      powershell -ExecutionPolicy Bypass -File scripts\build_setup.ps1 [-Iscc <path to ISCC.exe>]
#>
param([string]$Iscc)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
function Fail($m) { Write-Host "ERROR: $m" -ForegroundColor Red; exit 1 }

if (-not $Iscc) {
    $c = Get-Command ISCC.exe -ErrorAction SilentlyContinue
    if ($c) { $Iscc = $c.Source }
}
if (-not $Iscc) {
    foreach ($p in "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe", "$env:ProgramFiles\Inno Setup 6\ISCC.exe",
                   "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe") {
        if (Test-Path $p) { $Iscc = $p; break }
    }
}
if (-not $Iscc -or -not (Test-Path $Iscc)) { Fail 'ISCC.exe (Inno Setup 6) not found: install it or pass -Iscc' }

$m = Select-String -Path (Join-Path $root 'CMakeLists.txt') -Pattern 'project\(\S+\s+VERSION\s+([0-9.]+)'
if (-not $m) { Fail 'no project(... VERSION x.y.z) in CMakeLists.txt' }
$ver = $m.Matches[0].Groups[1].Value

foreach ($f in 'ets2_ats_vr_controls.dll', 'XR_APILAYER_JRDEVO_ets2_ats_vr_controls.json') {
    if (-not (Test-Path (Join-Path $root "build\Release\$f"))) { Fail "missing build\Release\$f (build Release first)" }
}

$stage = Join-Path $root 'build\setup\stage'
New-Item -ItemType Directory -Force $stage | Out-Null
$cfg = Join-Path $stage 'config.ini'
& cmd /c "git -C `"$root`" show HEAD:config.ini > `"$cfg`""          # (cmd's > keeps the bytes; PowerShell's would re-encode the UTF-8)
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $cfg) -or (Get-Item $cfg).Length -eq 0) { Fail 'git show HEAD:config.ini failed (the shipped config.ini comes from git)' }

& $Iscc /Q "/DAppVersion=$ver" "/DStageDir=$stage" (Join-Path $root 'installer\ets2_ats_vr_controls.iss')
if ($LASTEXITCODE -ne 0) { Fail "ISCC failed (exit $LASTEXITCODE)" }
$out = Join-Path $root "build\setup\ETS2-ATS-VR-Controls-Setup-v$ver.exe"
if (-not (Test-Path $out)) { Fail "no $out" }
Write-Host "built: $out" -ForegroundColor Green
exit 0
