<#
    uninstall.ps1 -- remove the ETS2 ATS VR Controls registration: the per-user copy (if any, as the user who started it), then
    HKLM (one UAC prompt unless it already runs as administrator): the OpenXR layer and the game's input plugin entries
    (HKLM\SOFTWARE\SCS Software\<game>\Plugins; an emptied Plugins key is removed too). The files stay.
      powershell -ExecutionPolicy Bypass -File scripts\uninstall.ps1
#>
param([switch]$Elevated)
$ErrorActionPreference = 'Stop'
if ([Environment]::Is64BitOperatingSystem -and -not [Environment]::Is64BitProcess) {   # 32-bit PowerShell: the 64-bit one
    $a = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"")
    if ($Elevated) { $a += '-Elevated' }
    & "$env:windir\sysnative\WindowsPowerShell\v1.0\powershell.exe" @a
    exit $LASTEXITCODE
}
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
foreach ($g in 'Euro Truck Simulator 2', 'American Truck Simulator') {
    $pk = "HKLM:\SOFTWARE\SCS Software\$g\Plugins"
    if (-not (Test-Path $pk)) { continue }
    $item = Get-Item $pk
    foreach ($n in $item.GetValueNames()) {
        if ((LeafOf ([string]$item.GetValue($n))) -ieq 'ets2_ats_vr_controls.dll') { Remove-ItemProperty -Path $pk -Name $n; Write-Host "removed plugin entry ($g): $n" }
    }
    $item = Get-Item $pk
    if (-not $item.GetValueNames().Count -and -not $item.SubKeyCount) { Remove-Item $pk; Write-Host "removed empty key: $pk" }
}
if ($Elevated) { Start-Sleep -Seconds 2 }
exit 0
