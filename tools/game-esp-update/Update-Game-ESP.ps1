param([switch]$ValidateOnly,[string]$TargetExe='', [switch]$NoLaunch)
$ErrorActionPreference = 'Stop'
try {
 $launch=Join-Path $PSScriptRoot 'tools/launch-app.ps1'
 if (!(Test-Path -LiteralPath $launch)) { throw 'Extract ALL files: launch-app.ps1 is missing.' }
 . $launch
 $payload = Join-Path $PSScriptRoot 'payload'
 $newExe = Join-Path $payload 'apex_visualizer.exe'
 $build = Get-Content -LiteralPath (Join-Path $payload 'BUILD-INFO.json') -Raw | ConvertFrom-Json
 if ((Get-FileHash -LiteralPath $newExe -Algorithm SHA256).Hash.ToLowerInvariant() -ne $build.exe_sha256) { throw 'Update executable SHA256 mismatch. Extract the complete original ZIP again.' }
 if ($ValidateOnly) { Write-Host 'Game-ESP update payload verified; no app files modified.'; exit 0 }
 if (!$TargetExe) {
  $running = @(Get-Process -Name apex_visualizer -ErrorAction SilentlyContinue | Where-Object { $_.Path })
  if ($running.Count -eq 1) { $TargetExe=$running[0].Path }
  if (!$TargetExe -and $running.Count -eq 0) { $known=Join-Path $env:USERPROFILE 'Downloads/Apex-Visualization-Compute-Fix/apex_visualizer.exe'; if (Test-Path -LiteralPath $known -PathType Leaf) { $TargetExe=$known } }
  if (!$TargetExe) {
   Add-Type -AssemblyName System.Windows.Forms
   $picker=New-Object System.Windows.Forms.OpenFileDialog
   try { $picker.Title='Select your CURRENT working apex_visualizer.exe';$picker.Filter='Apex demo|apex_visualizer.exe';$picker.InitialDirectory=Join-Path $env:USERPROFILE 'Downloads';if ($picker.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) { Write-Host 'Cancelled. Nothing changed.';exit 2 };$TargetExe=$picker.FileName } finally { $picker.Dispose() }
  }
 }
 $TargetExe=[IO.Path]::GetFullPath($TargetExe)
 if (!(Test-Path -LiteralPath $TargetExe -PathType Leaf) -or [IO.Path]::GetFileName($TargetExe) -ne 'apex_visualizer.exe') { throw 'Select the existing apex_visualizer.exe, not a folder or ZIP.' }
 $app=Split-Path -Parent $TargetExe
 Write-Host "Updating your existing app: $app" -ForegroundColor Cyan
 foreach ($process in @(Get-Process -Name apex_visualizer -ErrorAction SilentlyContinue)) {
  if ($process.Path -and [string]::Equals([IO.Path]::GetFullPath($process.Path),$TargetExe,[StringComparison]::OrdinalIgnoreCase)) {
   if (!$process.CloseMainWindow() -or !$process.WaitForExit(15000)) { throw 'The app is busy. Close it normally, then run Update-Game-ESP.cmd again.' }
  }
 }
 $backup=Join-Path $app ('update-backups/game-esp-'+[guid]::NewGuid().ToString())
 New-Item -ItemType Directory -Path $backup -Force | Out-Null
 $staged=$TargetExe+'.game-esp-update'
 Copy-Item -LiteralPath $newExe -Destination $staged -Force
 [IO.File]::Replace($staged,$TargetExe,(Join-Path $backup 'apex_visualizer.exe'))
 foreach ($name in @('BUILD-INFO.json','GAME-ESP-README.md','GAME-ESP-VALIDATION.md','HUD-Sample.mp4')) { Copy-Item -LiteralPath (Join-Path $payload $name) -Destination (Join-Path $app $name) -Force }
 $model=Join-Path $app 'models/compute_validation.onnx'
 if (!(Test-Path -LiteralPath $model)) { New-Item -ItemType Directory -Path (Split-Path $model) -Force | Out-Null;Copy-Item -LiteralPath (Join-Path $payload 'models/compute_validation.onnx') -Destination $model }
 Write-Host 'Game-ESP update installed. GPU libraries, saved settings and existing models were kept.' -ForegroundColor Green
 Write-Host "Old executable backup: $backup"
 if (!$NoLaunch) { $null=Start-VerifiedApexApp -Exe $TargetExe }
 Write-Host 'Open Game ESP > Browse enemy ONNX model. Select your local trained model, then open a recording or use your existing OBS capture.' -ForegroundColor Cyan
 exit 0
} catch { Write-Host "UPDATE STOPPED: $($_.Exception.Message)" -ForegroundColor Red;Write-Host 'Send a screenshot of this window.';exit 1 }
