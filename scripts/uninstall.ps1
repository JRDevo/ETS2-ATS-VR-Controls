<#
    uninstall.ps1 -- remove the VR Controller Keys registration (HKLM, one UAC prompt; also any per-user copy). The files stay.
      powershell -ExecutionPolicy Bypass -File scripts\uninstall.ps1
#>
param([switch]$Elevated)
$ErrorActionPreference = 'Stop'
$jsonName = 'XR_APILAYER_JRDEVO_controller_keys.json'
$admin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
$hives = if ($admin) { @('HKLM:') } else { @('HKCU:') }
foreach ($h in $hives) {
    $k = "$h\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit"
    if (-not (Test-Path $k)) { continue }
    foreach ($n in ((Get-Item $k).GetValueNames() | Where-Object { [System.IO.Path]::GetFileName($_) -ieq $jsonName })) {
        Remove-ItemProperty -Path $k -Name $n
        Write-Host "removed: $h $n"
    }
}
if (-not $admin) {
    Write-Host 'Requesting administrator rights (UAC) to remove the HKLM registration...'
    $p = Start-Process powershell -Verb RunAs -Wait -PassThru -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"", '-Elevated')
    exit $p.ExitCode
}
if ($Elevated) { Start-Sleep -Seconds 2 }
exit 0
