param(
 [string]$Exe = "$PSScriptRoot\..\apex_visualizer.exe",
 [string]$Model = '',
 [string]$InputProfile = '',
 [ValidateRange(1,86400)][int]$CpuSeconds = 10,
 [ValidateRange(1,86400)][int]$GpuSeconds = 10,
 [string]$Output = "$PSScriptRoot\..\model-results",
 [switch]$Sustained,
 [switch]$RequireCuda,
 [switch]$Warp
)
$ErrorActionPreference = 'Stop'
$exePath = (Resolve-Path -LiteralPath $Exe).Path
if (!$Model) { $Model = Join-Path (Split-Path $exePath) 'models/compute_validation.onnx' }
$modelPath = (Resolve-Path -LiteralPath $Model).Path
$run = Join-Path ([IO.Path]::GetFullPath($Output)) (Get-Date -Format 'yyyyMMdd-HHmmss-fff')
New-Item -ItemType Directory -Path $run -Force | Out-Null
Get-FileHash -LiteralPath $exePath -Algorithm SHA256 | ConvertTo-Json | Set-Content (Join-Path $run 'build.json')
Get-FileHash -LiteralPath $modelPath -Algorithm SHA256 | ConvertTo-Json | Set-Content (Join-Path $run 'model.json')
Get-CimInstance Win32_VideoController | Select-Object Name,DriverVersion | ConvertTo-Json | Set-Content (Join-Path $run 'adapters.json')
$arguments = @('--model-benchmark','--model',('"'+$modelPath+'"'),'--cpu-seconds',"$CpuSeconds",'--gpu-seconds',"$GpuSeconds",'--compute-iterations','20','--output',('"'+$run+'"'))
if ($InputProfile) { $profilePath = (Resolve-Path -LiteralPath $InputProfile).Path; $arguments += @('--input-profile',('"'+$profilePath+'"')); Copy-Item -LiteralPath $profilePath -Destination (Join-Path $run 'input-shapes.json') }
if ($Sustained) { $GpuSeconds = [Math]::Max(300,$GpuSeconds); $arguments += '--gpu-soak' }
if ($RequireCuda) { $arguments += '--require-cuda' }
if ($Warp) { $arguments += '--warp' }
$p = Start-Process -FilePath $exePath -ArgumentList $arguments -PassThru
if (-not $p.WaitForExit(([long]$CpuSeconds + $GpuSeconds + 600)*1000)) { $p.Kill(); throw "Timed out; GPU success not established: $run" }
$p.Refresh()
$report = Join-Path $run 'compute-report.json'
if (Test-Path $report) { Get-Content $report }
if ($p.ExitCode -ne 0) { throw "Benchmark did not pass requested execution criteria; exit $($p.ExitCode). Inspect $run" }
Write-Host "Actual results: $run. Inspect all inventory items: CPU-only success is not GPU success; memory-growth checks can remain inconclusive."
