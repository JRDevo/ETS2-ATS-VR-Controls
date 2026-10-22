<#
    uninstall.ps1 -- remove the ETS2 ATS VR Controls registration: the per-user copy (if any, as the user who started it), then
    HKLM (one UAC prompt unless it already runs as administrator). The files stay.
      powershell -ExecutionPolicy Bypass -File scripts\uninstall.ps1
#>
param([switch]$Elevated)
$ErrorActionPreference = 'Stop'
$jsonName = 'XR_APILAYER_JRDEVO_ets2_ats_vr_controls.json'
function LeafOf($p) { try { return [System.IO.Path]::GetFileName($p) } catch { return '' } }   # (odd foreign value names)
function RemoveFrom($hive) {
    $k = "$hive\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit"
    if (-not (Test-Path $k)) { return }
    foreach ($n in ((Get-Item $k).GetValueNames() | Where-Object { (LeafOf $_) -ieq $jsonName })) {
        Remove-ItemProperty -Path $k -Name $n
        Write-Host "removed: $hive $n"
    }
}
if (-not $Elevated) {
    RemoveFrom 'HKCU:'
    $admin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $admin) {
        Write-Host 'Requesting administrator rights (UAC) to remove the HKLM registration...'
        $p = Start-Process powershell -Verb RunAs -Wait -PassThru -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"", '-Elevated')
        exit $p.ExitCode
    }
}
RemoveFrom 'HKLM:'
if ($Elevated) { Start-Sleep -Seconds 2 }
exit 0
