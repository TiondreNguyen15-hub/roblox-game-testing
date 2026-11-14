param([switch]$ValidateOnly,[string]$TargetExe='')
$ErrorActionPreference='Stop'
function Run-Stage([string]$File,[string[]]$Extra=@()) {
 $arguments=@('-STA','-NoProfile','-ExecutionPolicy','Bypass','-File',('"'+$File+'"'))+$Extra
 $p=Start-Process -FilePath 'powershell.exe' -ArgumentList $arguments -Wait -PassThru -NoNewWindow
 if ($p.ExitCode -ne 0) { throw "Stage failed ($($p.ExitCode)): $([IO.Path]::GetFileName($File)). No later stage was started." }
}
try {
 $gpu=Join-Path $PSScriptRoot 'GPU-Reinstall/Install-GPU.ps1'
 $update=Join-Path $PSScriptRoot 'Update-Game-ESP.ps1'
 if (!(Test-Path -LiteralPath $gpu) -or !(Test-Path -LiteralPath $update)) { throw 'Extract ALL files from the ZIP first.' }
 if ($ValidateOnly) {
  Run-Stage $gpu @('-ValidateOnly')
  Run-Stage $update @('-ValidateOnly')
  Write-Host 'PASS combined launcher payload validation; nothing installed, no GPU execution claimed.'
  exit 0
 }
 $running=@(Get-Process -Name apex_visualizer -ErrorAction SilentlyContinue | Where-Object { $_.Path })
 $cache=Join-Path $env:LOCALAPPDATA 'ApexVisualization/last-launch-target.json'
 if (!$TargetExe -and $running.Count -eq 1) { $TargetExe=$running[0].Path }
 if (!$TargetExe -and $running.Count -gt 1) { throw 'Close the extra demos so only the copy to update is open.' }
 if (!$TargetExe -and (Test-Path -LiteralPath $cache)) { try { $saved=Get-Content -LiteralPath $cache -Raw | ConvertFrom-Json;if (Test-Path -LiteralPath $saved.exe -PathType Leaf) { $TargetExe=$saved.exe } } catch { } }
 if (!$TargetExe) {
  Add-Type -AssemblyName System.Windows.Forms
  $picker=New-Object System.Windows.Forms.OpenFileDialog
  try { $picker.Title='Select the installed apex_visualizer.exe that failed to reopen';$picker.Filter='Current demo|apex_visualizer.exe';$picker.InitialDirectory=Join-Path $env:USERPROFILE 'Downloads';if ($picker.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) { Write-Host 'Cancelled. No files changed.';exit 2 };$TargetExe=$picker.FileName } finally { $picker.Dispose() }
 }
 $target=[IO.Path]::GetFullPath($TargetExe)
 if (!(Test-Path -LiteralPath $target -PathType Leaf) -or [IO.Path]::GetFileName($target) -ne 'apex_visualizer.exe') { throw 'Select the installed apex_visualizer.exe.' }
 if ($target -match '[\\/]payload[\\/]|[\\/]update-backups[\\/]') { throw 'Select your installed app, not an update payload or backup.' }
 try { New-Item -ItemType Directory -Path (Split-Path -Parent $cache) -Force | Out-Null;@{exe=$target} | ConvertTo-Json | Set-Content -LiteralPath $cache -Encoding UTF8 } catch { Write-Host 'Could not remember target; continuing with explicit selected path.' }
 Write-Host "Repairing GPU dependencies and updating this exact copy: $target" -ForegroundColor Cyan
 Run-Stage $gpu @('-TargetExe',('"'+$target+'"'),'-NoLaunch')
 Run-Stage $update @('-TargetExe',('"'+$target+'"'))
 Write-Host 'Finished. In Game ESP select your model, enable Prefer CUDA and analyze a captured frame.' -ForegroundColor Green
 Write-Host 'Only CUDA EP execution profiled confirms this model actually executed GPU work. Installation is not verification.'
 exit 0
} catch { Write-Host "STOPPED: $($_.Exception.Message)" -ForegroundColor Red;exit 1 }
