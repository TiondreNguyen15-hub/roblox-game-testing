param([switch]$ValidateOnly)
$ErrorActionPreference = 'Stop'
try {
 $helper = Join-Path $PSScriptRoot 'tools/setup-gpu-runtime.ps1'
 $manifest = Join-Path $PSScriptRoot 'tools/cuda-dependencies.json'
 if (!(Test-Path -LiteralPath $helper) -or !(Test-Path -LiteralPath $manifest)) { throw 'Extract ALL files from the ZIP first, then run Reinstall-GPU.cmd.' }
 $packages = Get-Content -LiteralPath $manifest -Raw | ConvertFrom-Json
 if (@($packages.packages).Count -eq 0) { throw 'GPU dependency manifest is empty.' }
 if ($ValidateOnly) { Write-Host 'Launcher files and dependency manifest are valid. No files installed and no GPU execution tested.'; exit 0 }
 Write-Host 'Finding the demo you currently use...' -ForegroundColor Cyan
 $running = @(Get-Process -Name apex_visualizer -ErrorAction SilentlyContinue | Where-Object { $_.Path })
 $exe = ''
 if ($running.Count -eq 1) { $exe = $running[0].Path }
 if (!$exe -and $running.Count -eq 0) {
  $known = Join-Path $env:USERPROFILE 'Downloads/Apex-Visualization-Compute-Fix/apex_visualizer.exe'
  if (Test-Path -LiteralPath $known -PathType Leaf) { $exe = $known }
 }
 if (!$exe) {
  Add-Type -AssemblyName System.Windows.Forms
  $picker = New-Object System.Windows.Forms.OpenFileDialog
  try {
   $picker.Title = 'Select the apex_visualizer.exe you currently use'
   $picker.Filter = 'Apex demo (apex_visualizer.exe)|apex_visualizer.exe'
   $picker.InitialDirectory = Join-Path $env:USERPROFILE 'Downloads'
   if ($picker.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) { Write-Host 'Cancelled. Nothing installed.'; exit 2 }
   $exe = $picker.FileName
  } finally { $picker.Dispose() }
 }
 if (!(Test-Path -LiteralPath $exe -PathType Leaf)) { throw "Demo executable not found: $exe" }
 $appFolder = Split-Path -Parent ([IO.Path]::GetFullPath($exe))
 Write-Host "Installing GPU files into: $appFolder\runtime\gpu" -ForegroundColor Cyan
 foreach ($process in @(Get-Process -Name apex_visualizer -ErrorAction SilentlyContinue)) {
  if ($process.Path -and [string]::Equals([IO.Path]::GetFullPath($process.Path),[IO.Path]::GetFullPath($exe),[StringComparison]::OrdinalIgnoreCase)) {
   Write-Host 'Closing the demo normally so its runtime files can be updated...'
   if (!$process.CloseMainWindow() -or !$process.WaitForExit(15000)) { throw 'The demo is still open or busy. Close it yourself, then run Reinstall-GPU.cmd again. No forced termination was performed.' }
  }
 }
 Write-Host 'Preparing pinned ONNX/CUDA/cuDNN files. About 1.4 GB may download; existing cached packages will be reused.' -ForegroundColor Yellow
 & $helper -Destination (Join-Path $appFolder 'runtime/gpu') -InstallDependencies
 Write-Host 'GPU runtime files installed. Reopening your demo...' -ForegroundColor Green
 Start-Process -FilePath $exe -WorkingDirectory $appFolder
 Write-Host 'In the app: System > Run CUDA + ONNX validation. Installation alone does not verify GPU execution.' -ForegroundColor Cyan
 exit 0
} catch {
 Write-Host "INSTALLATION STOPPED: $($_.Exception.Message)" -ForegroundColor Red
 Write-Host 'Send a screenshot of this window. Do not download individual DLLs from DLL websites.'
 exit 1
}
