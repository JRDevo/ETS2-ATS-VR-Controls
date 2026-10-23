<#
    run_e2e.ps1 -- end to end: tests\fake_game.cpp (through the real OpenXR loader) -> ets2_ats_vr_controls.dll (as an EXPLICIT layer:
    XR_API_LAYER_PATH + XR_ENABLE_API_LAYERS, nothing registered, no admin) -> tests\mock_runtime.cpp (XR_RUNTIME_JSON) with
    scripted controller input (VRC_MOCK_INPUT). The layer runs in test mode (VRC_TEST_EVENTS): it writes its key / mouse events
    to a file instead of sending them, so no real input reaches Windows. CPU only, no headset.
      ats        the game suggests only simple_controller (ATS / ETS2): touch gets the game's bindings translated + ours, the
                 game still sees its select on the trigger, our trigger = left click, A = Enter, left stick = mouse, right grip
                 + aim turn = air mouse, B combo from config
      touch      the game suggests touch itself: ours merged into it
      nosync     the game attaches but never syncs: the layer syncs its own set
      noinput    the game has no input at all: the layer attaches its own set after VRC_TEST_SELF_ATTACH_S
      notallowed the exe is not in 'apps': no events, no log file
      stall      the game stops syncing for 1.2 s: the layer syncs itself WITH the game's set; the game's input never goes inactive
      slowsync   the game syncs every 4th frame: read from its syncs, no own sync, same mouse distance
      nodestroy  the game ends with xrDestroyInstance only while the trigger is held: released
      vive       Vive wands: the game's right menu = our B, left to the game with a warning
      unfocus    the session leaves FOCUSED while the trigger is held: released at once, pressed again after
      restore    the runtime refuses our bindings merged into the game's Touch suggestion: the game's own is put back
      menu       a D3D11 game with VR EyeCapture's set (both stick clicks, shared): both grips + the left stick click held 1 s open the VR menu (quad layers on D3D11 swapchains), the right hand
                 points at the Mouse tab and at "air mouse: toggled on" (written to config.ini), the game never sees those
                 trigger pulls, a short stick click alone is still the horn (at once), the combo again closes it
    Also checked: the layer never syncs alone while the game syncs (mock "solo" syncs = 0); own sync only after 2 s of FOCUSED.
    Windows PowerShell 5.1 compatible:  powershell -ExecutionPolicy Bypass -File tests\run_e2e.ps1 [-Only ats]
#>
param([string]$Only = '', [switch]$Register)
# -Register (CI only: a throwaway runner that runs ELEVATED): the OpenXR loader ignores XR_RUNTIME_JSON and XR_API_LAYER_PATH
# in an elevated process, so the mock runtime is made the active runtime and the layer an explicit layer in HKLM instead.
# Never use it on a PC with a real VR runtime (it replaces the active OpenXR runtime).
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$rel = Join-Path $root 'build\Release'
$game = Join-Path $rel 'fake_game.exe'
foreach ($f in $game, (Join-Path $rel 'ets2_ats_vr_controls.dll'), (Join-Path $rel 'mock_runtime.dll')) {
    if (-not (Test-Path $f)) { Write-Host "missing $f (build first)" -ForegroundColor Red; exit 2 }
}
$work = Join-Path $root 'build\e2e'
$env:XR_RUNTIME_JSON = Join-Path $rel 'mock_runtime.json'
# the layer in a folder of its own: the loader reads EVERY json in XR_API_LAYER_PATH as a layer manifest
$layerDir = Join-Path $work 'layer'
New-Item -ItemType Directory -Force $layerDir | Out-Null
Copy-Item (Join-Path $rel 'ets2_ats_vr_controls.dll'), (Join-Path $rel 'XR_APILAYER_JRDEVO_ets2_ats_vr_controls.json') $layerDir -Force
$env:XR_API_LAYER_PATH = $layerDir
if ($Register) {
    $k = 'HKLM:\SOFTWARE\Khronos\OpenXR'
    New-Item -Force "$k\ApiLayers\Explicit" | Out-Null
    Set-ItemProperty $k -Name ActiveRuntime -Value $env:XR_RUNTIME_JSON
    New-ItemProperty "$k\ApiLayers\Explicit" -Name (Join-Path $layerDir 'XR_APILAYER_JRDEVO_ets2_ats_vr_controls.json') -Value 0 -PropertyType DWord -Force | Out-Null
    Write-Host "registered (elevated CI): active runtime = mock, explicit layer = $layerDir"
}
$env:XR_ENABLE_API_LAYERS = 'XR_APILAYER_JRDEVO_ets2_ats_vr_controls'
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
    $env:VRC_CONFIG = $cfg
    $env:VRC_LOG_DIR = Join-Path $d 'logs'
    $env:VRC_TEST_EVENTS = Join-Path $d 'events.txt'
    $env:VRC_MOCK_REPORT = Join-Path $d 'mock.txt'
    $env:VRC_MOCK_INPUT = $script
    $env:VRC_TEST_SELF_ATTACH_S = $null
    $env:VRC_MOCK_UNFOCUS = $null
    $env:VRC_MOCK_REFUSE_MERGED = $null
    $env:VRC_MOCK_PROFILE = $null
    $env:VRC_MOCK_DUMP = $null
    if ($extraEnv) { foreach ($k in $extraEnv.Keys) { Set-Item "env:$k" $extraEnv[$k] } }
    Write-Host "`n=== $name" -ForegroundColor Cyan
    $out = Join-Path $d 'game.txt'
    $p = Start-Process -FilePath $game -ArgumentList (@("$seconds") + $gameArgs) -PassThru -NoNewWindow -RedirectStandardOutput $out
    $null = $p.Handle                                    # (keeps the exit code readable after the process ends)
    if (-not $p.WaitForExit(($seconds + 20) * 1000)) { $p.Kill(); Write-Host 'TIMEOUT' -ForegroundColor Red }
    $p.WaitForExit()
    $r = @{
        exit   = $p.ExitCode
        events = if (Test-Path $env:VRC_TEST_EVENTS) { @(Get-Content $env:VRC_TEST_EVENTS) } else { @() }
        game   = if (Test-Path $out) { Get-Content $out -Raw } else { '' }
        mock   = if (Test-Path $env:VRC_MOCK_REPORT) { Get-Content $env:VRC_MOCK_REPORT -Raw } else { '' }
        log    = if (Test-Path (Join-Path $env:VRC_LOG_DIR 'ets2_ats_vr_controls.log')) { Get-Content (Join-Path $env:VRC_LOG_DIR 'ets2_ats_vr_controls.log') -Raw } else { '' }
        logDir = $env:VRC_LOG_DIR
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
    Check 'ats' ($r.mock -match 'attach input\(0\) ets2_ats_vr_controls\(0\)') 'our set attached after the game''s, same priority'
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
    Check 'ats' ($r.mock -match 'syncs [1-9]\d* solo 0') 'the layer never syncs alone while the game syncs (the game''s set never goes inactive)'
    Check 'ats' ($r.game -match 'game input inactive reads: 0') 'the game''s input never went inactive'
}
# ---- stall: the game stops syncing 2.3-3.5 s (loading / hitch); the left stick (2.4-2.9) must still move the mouse, through the
# layer's own sync WITH the game's set, so the game's input never goes inactive
if (-not $Only -or $Only -eq 'stall') {
    $r = RunCase 'stall' 'fake_game.exe' @('--stall', '2.3-3.5') 4
    $m = SumMoves $r.events
    Write-Host "  moves total: $($m[0]), $($m[1])"
    Check 'stall' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'stall' ($r.log -match 'the layer syncs itself \(the game''s last sync is older than 0\.25 s; 1 of the game''s sets kept active\)') 'log: own sync with the game''s set'
    # stick 0.5 s (~450 px, all inside the stall) + the air mouse turn up to the end at 4 s (~150-250 px)
    Check 'stall' (($m[0] -gt 450) -and ($m[0] -lt 800)) 'the stick moved the mouse during the stall (total ~550-700 px)'
    Check 'stall' ($r.game -match 'game input inactive reads: 0') 'the game''s input never went inactive'
    Check 'stall' ($r.mock -match 'syncs [1-9]\d* solo 0') 'no sync without the game''s set'
}
# ---- slowsync: the game syncs every 4th frame (~44 ms): the layer reads from the game's syncs, no own sync, same mouse distance
if (-not $Only -or $Only -eq 'slowsync') {
    $r = RunCase 'slowsync' 'fake_game.exe' @('--sync-every', '4') 6
    $m = SumMoves $r.events
    Write-Host "  moves total: $($m[0]), $($m[1])"
    Check 'slowsync' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'slowsync' ($r.log -notmatch 'the layer syncs itself') 'no own sync'
    Check 'slowsync' (($m[0] -gt 550) -and ($m[0] -lt 850)) 'stick + air mouse distance as with a sync every frame'
    Check 'slowsync' ((Buttons $r.events) -match 'mouse 0 down \| mouse 0 up \| key 01c down \| key 01c up') 'trigger + A'
    Check 'slowsync' ($r.game -match 'game input inactive reads: 0') 'the game''s input never went inactive'
}
# ---- nodestroy: the trigger is held when the game ends with xrDestroyInstance only -> released at the instance destroy
if (-not $Only -or $Only -eq 'nodestroy') {
    $r = RunCase 'nodestroy' 'fake_game.exe' @('--no-destroy-session') 1.3
    $b = Buttons $r.events
    Write-Host "  events: $b"
    Check 'nodestroy' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'nodestroy' ($b -eq 'mouse 0 down | mouse 0 up') 'the held click is released when the instance ends'
}
# ---- vive: Vive wands; the game's menu (right) = our B: left to the game, with a warning because config.ini maps b
if (-not $Only -or $Only -eq 'vive') {
    $r = RunCase 'vive' 'fake_game.exe' @() 5.5 @{ VRC_MOCK_PROFILE = '/interaction_profiles/htc/vive_controller' }
    Check 'vive' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'vive' ($r.mock -match 'profile /interaction_profiles/htc/vive_controller') 'the runtime picked Vive'
    Check 'vive' ($r.log -match ' W htc/vive_controller: config\.ini b will do nothing') 'log: warning for the lost B mapping'
    Check 'vive' ((Buttons $r.events) -match '^mouse 0 down \| mouse 0 up$') 'trigger works, nothing else (no A on a Vive wand)'
}
# ---- unfocus: the right trigger held 1.0-1.5 s, the session not FOCUSED 1.2-1.35 s -> the click is released at once, pressed again
if (-not $Only -or $Only -eq 'unfocus') {
    $r = RunCase 'unfocus' 'fake_game.exe' @() 2.5 @{ VRC_MOCK_UNFOCUS = '1.2-1.35' }
    $b = Buttons $r.events
    Write-Host "  events: $b"
    Check 'unfocus' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'unfocus' ($b -match '^mouse 0 down \| mouse 0 up \| mouse 0 down \| mouse 0 up') 'released on focus loss, pressed again after it'
    Check 'unfocus' ($r.log -match 'not focused: no input sent') 'log: focus loss'
}
# ---- index: the controllers are Valve Index; the game's translated menu is b/click there, so our B is left to the game
if (-not $Only -or $Only -eq 'index') {
    $r = RunCase 'index' 'fake_game.exe' @() 5.5 @{ VRC_MOCK_PROFILE = '/interaction_profiles/valve/index_controller' }
    $b = Buttons $r.events
    Write-Host "  events: $b"
    Check 'index' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'index' ($r.mock -match 'profile /interaction_profiles/valve/index_controller') 'the runtime picked Index'
    Check 'index' ($r.mock -match 'suggest /interaction_profiles/valve/index_controller \d+:.*menu=/user/hand/right/input/b/click') "Index carries the game's menu on B"
    Check 'index' ($r.log -match 'valve/index_controller: .*b/click.* the game''s menu button') 'log: our B left to the game'
    Check 'index' ($b -notmatch 'key 02a') 'B sends no mod key (the game''s menu only)'
    Check 'index' ($b -match 'mouse 0 down \| mouse 0 up \| key 01c down \| key 01c up') 'trigger + A still work'
}
# ---- restore: the runtime refuses our bindings merged into the game's Touch suggestion -> the game's own is put back
if (-not $Only -or $Only -eq 'restore') {
    $r = RunCase 'restore' 'fake_game.exe' @('--touch') 2.5 @{ VRC_MOCK_REFUSE_MERGED = '/interaction_profiles/oculus/touch_controller' }
    Check 'restore' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'restore' ($r.log -match 'refused our bindings on oculus/touch_controller .* re-suggested \(0\)') 'log: the game''s own Touch bindings re-suggested'
    $last = @($r.mock -split "`n" | Where-Object { $_ -match '^suggest /interaction_profiles/oculus/touch_controller \d+:' }) | Select-Object -Last 1
    Check 'restore' ($last -match 'touch_controller 3:') "the last Touch suggestion is the game's 3 bindings ($last)"
    Check 'restore' ($r.game -match 'game select right down') 'the game keeps its select'
}
# ---- menu: the VR menu on a D3D11 session. The menu opens 0.75 m ahead, 8 deg down: the Mouse tab (2nd of 4) is at aim yaw -5.4 / pitch 5,
# "Toggled on" (air mouse) at yaw 16.1 / pitch -14.2 (vr_menu.cpp layout; mock: + yaw = right)
if (-not $Only -or $Only -eq 'menu') {
    $menuScript = '1.0-2.6:/user/hand/left/input/squeeze/value=1;1.0-2.6:/user/hand/right/input/squeeze/value=1;' +
                  '1.2-2.6:/user/hand/left/input/thumbstick/click=1;' +
                  '2.6-3.6:/user/hand/right/input/aim/pose=-5.4,5,-5.4,5;' +
                  '3.0-3.3:/user/hand/right/input/trigger/value=0.9;3.0-3.3:/user/hand/right/input/select/click=1;' +
                  '3.7-4.6:/user/hand/right/input/aim/pose=16.1,-14.2,16.1,-14.2;' +
                  '4.0-4.3:/user/hand/right/input/trigger/value=0.9;4.0-4.3:/user/hand/right/input/select/click=1;' +
                  '5.0-5.1:/user/hand/right/input/thumbstick/click=1;' +
                  '5.5-7.0:/user/hand/left/input/squeeze/value=1;5.5-7.0:/user/hand/right/input/squeeze/value=1;' +
                  '5.6-7.0:/user/hand/left/input/thumbstick/click=1'
    $d = Join-Path $work 'menu'
    $r = RunCase 'menu' 'fake_game.exe' @('--d3d11', '--eyecapture') 7.5 @{ VRC_MOCK_INPUT = $menuScript; VRC_MOCK_DUMP = $d }
    $b = Buttons $r.events
    Write-Host "  events: $b"
    $cfgText = Get-Content (Join-Path $d 'config.ini') -Raw
    Check 'menu' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'menu' ($r.log -notmatch 'thumbstick/click is the game''s menu') 'the stick clicks are shared with VR EyeCapture''s binding'
    Check 'menu' ($r.log -match 'oculus/touch_controller: /user/hand/left/input/menu/click is the game''s menu button') 'only the game''s menu left to it'
    Check 'menu' ($r.log -match 'VR menu open') 'log: the combo opened the menu'
    Check 'menu' ($r.log -match 'right hand presses menu tab 1') 'log: Mouse tab pressed'
    Check 'menu' ($r.log -match 'right hand presses menu setting 41') 'log: air mouse "toggled on" pressed'
    Check 'menu' ($r.log -match 'VR menu closed') 'log: the combo closed it'
    Check 'menu' ($cfgText -match '(?m)^air_mode = toggle') 'config.ini: air_mode = toggle written'
    Check 'menu' ($cfgText -match '(?m)^b = shift\+f') 'config.ini: the test''s own b line kept'
    Check 'menu' ($r.mock -match 'swapchain 1000x800') 'the menu swapchain made on the game''s D3D11 device'
    # (the left controller is not scripted: it points straight ahead, at the menu too -- 2 dots)
    Check 'menu' ($r.mock -match 'endframe quads 3') 'frames with 3 quads (menu + both pointer dots)'
    Check 'menu' ($r.mock -match 'endframe quads 0[\s\S]*endframe quads 3[\s\S]*endframe quads 0') 'no quads before / after the menu'
    Check 'menu' ($r.mock -notmatch 'REFUSED \(quad|endframe REFUSED') 'every frame with our layers accepted'
    Check 'menu' ($r.mock -notmatch 'session destroyed with') 'our swapchains destroyed before the session'
    Check 'menu' (Test-Path (Join-Path $d 'quad_1000x800.bmp')) 'the menu picture reached the swapchain (quad_1000x800.bmp)'
    Check 'menu' ($r.game -notmatch 'game select right down at [34]\.') 'the game never saw the trigger pulls on the menu'
    Check 'menu' ($b -notmatch 'mouse 0') 'no left click from the menu trigger pulls'
    Check 'menu' ($b -eq 'key 023 down | key 023 up') "only the short right stick click (horn H) went out ($b)"
    Check 'menu' ($r.log -notmatch ' E ') 'log: no errors'
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
    # own sync only after 2 s of FOCUSED without any game sync: the trigger at 1.0-1.5 s is not seen, B at 4.8 s is
    $r = RunCase 'nosync' 'fake_game.exe' @('--no-sync') 5.5
    $b = Buttons $r.events
    Check 'nosync' ($r.exit -eq 0) "game exit 0 -- $($r.exit)"
    Check 'nosync' ($r.log -match 'the layer syncs itself \(the game attached its input but has not synced in 2 s of FOCUSED; 1 of the game''s sets kept active') 'log: own sync with the game''s set'
    Check 'nosync' ($r.mock -match 'syncs [1-9]\d* solo 0') 'no sync without the game''s set'
    Check 'nosync' ($b -notmatch 'mouse 0') 'no own sync in the first 2 s (the trigger at 1.0 s not sent)'
    Check 'nosync' ($b -match 'key 02a down \| key 021 down \| key 021 up \| key 02a up') 'B -> Shift+F with our own sync'
}
# ---- noinput
if (-not $Only -or $Only -eq 'noinput') {
    $r = RunCase 'noinput' 'fake_game.exe' @('--no-input') 3 @{ VRC_TEST_SELF_ATTACH_S = '0.5' }
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
