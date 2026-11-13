param(
 [ValidateRange(0,86400)][int]$Seconds = 30,
 [ValidateRange(1,86400)][int]$Iterations = 200,
 [string]$Exe = "$PSScriptRoot\..\apex_visualizer.exe",
 [string]$Output = "$PSScriptRoot\..\compute-results",
 [switch]$RequireCuda,
 [switch]$Warp
)
$ErrorActionPreference = 'Stop'
$exePath = (Resolve-Path $Exe).Path
$run = Join-Path ([IO.Path]::GetFullPath($Output)) (Get-Date -Format 'yyyyMMdd-HHmmss-fff')
New-Item -ItemType Directory -Path $run -Force | Out-Null
Get-FileHash $exePath -Algorithm SHA256 | ConvertTo-Json | Set-Content (Join-Path $run 'build.json')
Get-CimInstance Win32_VideoController | Select-Object Name,DriverVersion,PNPDeviceID | ConvertTo-Json | Set-Content (Join-Path $run 'display-adapters.json')
Get-CimInstance Win32_Processor | Select-Object Name,NumberOfLogicalProcessors | ConvertTo-Json | Set-Content (Join-Path $run 'cpu.json')
Get-CimInstance Win32_OperatingSystem | Select-Object Caption,Version,BuildNumber | ConvertTo-Json | Set-Content (Join-Path $run 'os.json')
$arguments = @('--compute-test','--compute-seconds',"$Seconds",'--compute-iterations',"$Iterations",'--output',('"' + $run + '"'))
if ($RequireCuda) { $arguments += '--require-cuda' }
if ($Warp) { $arguments += '--warp' }
$model = Join-Path (Split-Path $exePath) 'models/compute_validation.onnx'
if (Test-Path $model) { Get-FileHash $model -Algorithm SHA256 | ConvertTo-Json | Set-Content (Join-Path $run 'model.json') }
$p = Start-Process -FilePath $exePath -ArgumentList $arguments -PassThru
# Seconds is the MINIMUM for each of up to three available phases; slow iteration
# counts can take longer. The bounded allowance below catches hangs, not TDR recovery.
if (-not $p.WaitForExit(([long]$Seconds * 3 + 600) * 1000)) { $p.Kill(); throw "Compute run timed out: $run" }
$p.Refresh()
$log = Join-Path $env:LOCALAPPDATA 'ApexVisualization/apex.log'
if (Test-Path $log) { Copy-Item $log (Join-Path $run 'application.log') }
$report = Join-Path $run 'compute-report.json'
if (Test-Path $report) { Get-Content $report }
if ($p.ExitCode -ne 0) { throw "Compute validation exited $($p.ExitCode); inspect $run. This does not establish GPU acceleration." }
Write-Host "Actual results: $run. CPU success without -RequireCuda does not mean CUDA passed."
