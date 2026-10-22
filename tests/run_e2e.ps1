<#
    run_e2e.ps1 -- end to end: tests\fake_game.cpp (through the real OpenXR loader) -> controller_keys.dll (as an EXPLICIT layer:
    XR_API_LAYER_PATH + XR_ENABLE_API_LAYERS, nothing registered, no admin) -> tests\mock_runtime.cpp (XR_RUNTIME_JSON) with
    scripted controller input (CK_MOCK_INPUT). The layer runs in test mode (CK_TEST_EVENTS): it writes its key / mouse events
    to a file instead of sending them, so no real input reaches Windows. CPU only, no headset.
      ats        the game suggests only simple_controller (ATS / ETS2): touch gets the game's bindings translated + ours, the
                 game still sees its select on the trigger, our trigger = left click, A = Enter, left stick = mouse, right grip
                 + aim turn = air mouse, B combo from config
      touch      the game suggests touch itself: ours merged into it
      nosync     the game attaches but never syncs: the layer syncs its own set
      noinput    the game has no input at all: the layer attaches its own set after CK_TEST_SELF_ATTACH_S
      notallowed the exe is not in 'apps': no events, no log file
    Windows PowerShell 5.1 compatible:  powershell -ExecutionPolicy Bypass -File tests\run_e2e.ps1 [-Only ats]
#>
param([string]$Only = '')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$rel = Join-Path $root 'build\Release'
$game = Join-Path $rel 'fake_game.exe'
foreach ($f in $game, (Join-Path $rel 'controller_keys.dll'), (Join-Path $rel 'mock_runtime.dll')) {
    if (-not (Test-Path $f)) { Write-Host "missing $f (build first)" -ForegroundColor Red; exit 2 }
}
$work = Join-Path $root 'build\e2e'
$env:XR_RUNTIME_JSON = Join-Path $rel 'mock_runtime.json'
# the layer in a folder of its own: the loader reads EVERY json in XR_API_LAYER_PATH as a layer manifest
$layerDir = Join-Path $work 'layer'
New-Item -ItemType Directory -Force $layerDir | Out-Null
Copy-Item (Join-Path $rel 'controller_keys.dll'), (Join-Path $rel 'XR_APILAYER_JRDEVO_controller_keys.json') $layerDir -Force
$env:XR_API_LAYER_PATH = $layerDir
$env:XR_ENABLE_API_LAYERS = 'XR_APILAYER_JRDEVO_controller_keys'
$env:XR_LOADER_DEBUG = $null
# every implicit layer installed on this PC (Virtual Desktop, VectorXR, VR EyeCapture, ...) off through its own
# disable_environment: they expect a real runtime. Only the build copy of ours runs (explicit, above).
foreach ($hive in 'HKLM:', 'HKCU:') {
    $k = "$hive\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit"
    if (-not (Test-Path $k)) { continue }
    foreach ($json in (Get-Item $k).GetValueNames()) {
        try {
            $m = Get-Content $json -Raw | ConvertFrom-Json
            $de = $m.api_layer.disable_environment
            if ($de) { Set-Item "env:$de" '1'; Write-Host "implicit layer off for the test: $($m.api_layer.name) ($de=1)" }
        } catch { Write-Host "implicit layer manifest not readable: $json" }
    }
}
$results = @()

# right trigger 1.0-1.5; A 1.8-2.1; left stick right 2.4-2.9; right grip 3.2-4.2 with the aim turning 0 -> 10 deg right over
# 3.4-4.0 (then held at 10 until 4.2); B 4.5-4.7 (config: shift+f)
$script = '1.0-1.5:/user/hand/right/input/trigger/value=0.9;1.0-1.5:/user/hand/right/input/select/click=1;' +
          '1.8-2.1:/user/hand/right/input/a/click=1;' +
          '2.4-2.9:/user/hand/left/input/thumbstick=1,0;' +
          '3.2-4.2:/user/hand/right/input/squeeze/value=1;' +
          '3.4-4.0:/user/hand/right/input/aim/pose=0,0,10,0;4.0-4.6:/user/hand/right/input/aim/pose=10,0;' +
          '4.8-5.0:/user/hand/right/input/b/click=1'

function RunCase($name, $apps, $gameArgs, $seconds, [hashtable]$extraEnv) {
    $d = Join-Path $work $name
    if (Test-Path $d) { Remove-Item $d -Recurse -Force }
    New-Item -ItemType Directory -Force $d | Out-Null
    $cfg = Join-Path $d 'config.ini'
    "apps = $apps`nb = shift+f`nlog_inputs = 1`n" | Set-Content -Encoding ASCII $cfg
    $env:CK_CONFIG = $cfg
    $env:CK_LOG_DIR = Join-Path $d 'logs'
    $env:CK_TEST_EVENTS = Join-Path $d 'events.txt'
    $env:CK_MOCK_REPORT = Join-Path $d 'mock.txt'
    $env:CK_MOCK_INPUT = $script
    $env:CK_TEST_SELF_ATTACH_S = $null
    if ($extraEnv) { foreach ($k in $extraEnv.Keys) { Set-Item "env:$k" $extraEnv[$k] } }
    Write-Host "`n=== $name" -ForegroundColor Cyan
    $out = Join-Path $d 'game.txt'
    $p = Start-Process -FilePath $game -ArgumentList (@("$seconds") + $gameArgs) -PassThru -NoNewWindow -RedirectStandardOutput $out
    $null = $p.Handle                                    # (keeps the exit code readable after the process ends)
    if (-not $p.WaitForExit(($seconds + 20) * 1000)) { $p.Kill(); Write-Host 'TIMEOUT' -ForegroundColor Red }
    $p.WaitForExit()
    $r = @{
        exit   = $p.ExitCode
        events = if (Test-Path $env:CK_TEST_EVENTS) { @(Get-Content $env:CK_TEST_EVENTS) } else { @() }
        game   = if (Test-Path $out) { Get-Content $out -Raw } else { '' }
        mock   = if (Test-Path $env:CK_MOCK_REPORT) { Get-Content $env:CK_MOCK_REPORT -Raw } else { '' }
        log    = if (Test-Path (Join-Path $env:CK_LOG_DIR 'controller_keys.log')) { Get-Content (Join-Path $env:CK_LOG_DIR 'controller_keys.log') -Raw } else { '' }
        logDir = $env:CK_LOG_DIR
    }
    return $r
}

function Check($case, $cond, $what) {
    $script:results += [pscustomobject]@{ case = $case; ok = [bool]$cond; what = $what }
    if ($cond) { Write-Host "  ok    $what" -ForegroundColor Green } else { Write-Host "  FAIL  $what" -ForegroundColor Red }
}
function SumMoves($events) {
    $x = 0; $y = 0
    foreach ($e in $events) { if ($e -match '^move (-?\d+) (-?\d+)$') { $x += [int]$Matches[1]; $y += [int]$Matches[2] } }
    return @($x, $y)
}
function Buttons($events) { return (@($events | Where-Object { $_ -notmatch '^move ' }) -join ' | ') }

# ---- ats
if (-not $Only -or $Only -eq 'ats') {
    $r = RunCase 'ats' 'fake_game.exe' @() 6
    $b = Buttons $r.events
    Write-Host "  events: $b"
    Check 'ats' ($r.exit -eq 0) "game exit 0 (all its calls succeeded) -- $($r.exit)"
    Check 'ats' ($r.mock -match 'profile /interaction_profiles/oculus/touch_controller') 'the runtime picked Touch'
    Check 'ats' ($r.mock -match 'suggest /interaction_profiles/oculus/touch_controller \d+:.*select=/user/hand/right/input/trigger/value') "Touch carries the game's select on the trigger"
    Check 'ats' ($r.mock -match 'attach input\(0\) controller_keys\(0\)') 'our set attached after the game''s, same priority'
    Check 'ats' ($r.mock -notmatch 'REFUSED \(bad binding') 'no binding path refused'
    Check 'ats' ($r.mock -match 'hp/mixed_reality_controller REFUSED \(profile not supported\)') 'HP profile refused by this runtime (skipped, not fatal)'
    Check 'ats' ($r.game -match 'game select right down at (0\.9|1\.[0-2])') 'the game still sees its select (right trigger)'
    Check 'ats' ($b -match 'mouse 0 down \| mouse 0 up') 'right trigger -> left click down + up'
    Check 'ats' ($b -match 'key 01c down \| key 01c up') 'A -> Enter'
    Check 'ats' ($b -match 'key 02a down \| key 021 down \| key 021 up \| key 02a up') 'B -> Shift+F (config), released in reverse'
    $m = SumMoves $r.events
    Write-Host "  moves total: $($m[0]), $($m[1])"
    # stick: 0.5 s full right at 900 px/s = ~450; air: 10 deg at 25 px/deg = 250
    Check 'ats' (($m[0] -gt 550) -and ($m[0] -lt 850) -and ([math]::Abs($m[1]) -lt 10)) 'stick 0.5 s (~450 px) + air mouse 10 deg (~250 px) to the right'
    Check 'ats' ($r.log -match 'only khr/simple_controller') 'log: the simple_controller translation'
    Check 'ats' ($r.log -match 'left controller ACTIVE \(oculus/touch_controller\)') 'log: controllers active on Touch'
    Check 'ats' ($r.log -match 'first game xrSyncActions with our action set appended') 'log: our set rides the game''s sync'
    Check 'ats' ($r.log -notmatch ' E ') 'log: no errors'
    $downs = @($r.events | Where-Object { $_ -match ' down$' }).Count; $ups = @($r.events | Where-Object { $_ -match ' up$' }).Count
    Check 'ats' ($downs -eq $ups) "every down has its up ($downs / $ups)"
}
# ---- touch
if (-not $Only -or $Only -eq 'touch') {
    $r = RunCase 'touch' 'fake_game.exe' @('--touch') 4
    Check 'touch' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'touch' ($r.log -match 'on oculus/touch_controller \(merged into the game''s 3 bindings') 'ours merged into the game''s Touch suggestion'
    Check 'touch' ($r.log -notmatch 'only khr/simple_controller') 'no translation (the game has a real profile)'
    Check 'touch' ($r.game -match 'game select right down') 'the game sees its select'
    Check 'touch' ((Buttons $r.events) -match 'mouse 0 down \| mouse 0 up') 'right trigger -> left click'
}
# ---- nosync
if (-not $Only -or $Only -eq 'nosync') {
    $r = RunCase 'nosync' 'fake_game.exe' @('--no-sync') 3
    Check 'nosync' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'nosync' ($r.log -match 'the layer syncs its action set itself \(no game xrSyncActions') 'log: own sync'
    Check 'nosync' ((Buttons $r.events) -match 'key 01c down \| key 01c up') 'A -> Enter with our own sync'
}
# ---- noinput
if (-not $Only -or $Only -eq 'noinput') {
    $r = RunCase 'noinput' 'fake_game.exe' @('--no-input') 3 @{ CK_TEST_SELF_ATTACH_S = '0.5' }
    Check 'noinput' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'noinput' ($r.log -match 'the game has no controller input of its own') 'log: self attach'
    Check 'noinput' ($r.mock -match 'profile /interaction_profiles/oculus/touch_controller') 'Touch picked from our own suggestions'
    Check 'noinput' ((Buttons $r.events) -match 'mouse 0 down \| mouse 0 up \| key 01c down \| key 01c up') 'trigger + A work'
}
# ---- notallowed
if (-not $Only -or $Only -eq 'notallowed') {
    $r = RunCase 'notallowed' 'eurotrucks2.exe, amtrucks.exe' @() 2.5
    Check 'notallowed' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'notallowed' ($r.events.Count -eq 0) 'no events'
    Check 'notallowed' (-not (Test-Path $r.logDir)) 'no log written'
    Check 'notallowed' ($r.mock -match 'attach input\(0\)$' -or $r.mock -match "attach input\(0\)`n") 'only the game''s set attached'
    Check 'notallowed' ($r.game -match 'game select right down') 'the game works as without the layer'
}

$bad = @($results | Where-Object { -not $_.ok })
Write-Host ""
if ($bad.Count) { Write-Host "$($bad.Count) of $($results.Count) checks FAILED" -ForegroundColor Red; exit 1 }
Write-Host "all $($results.Count) checks passed" -ForegroundColor Green
exit 0
