param(
 [ValidateRange(1,86400)][int]$Seconds = 120,
 [ValidateRange(0,300)][int]$Warmup = 5,
 [ValidateRange(30,360)][int]$Cap = 144,
 [string]$Exe = "$PSScriptRoot\..\apex_visualizer.exe",
 [string]$Output = "$PSScriptRoot\..\benchmark-results",
 [switch]$Warp
)
$ErrorActionPreference = 'Stop'
$exePath = (Resolve-Path $Exe).Path
# Use a new directory per run; do not overwrite previous measurements.
$run = Join-Path ([IO.Path]::GetFullPath($Output)) (Get-Date -Format 'yyyyMMdd-HHmmss-fff')
New-Item -ItemType Directory -Path $run -Force | Out-Null
$arguments = @('--benchmark', "$Seconds", '--warmup', "$Warmup", '--cap', "$Cap", '--entities', '64', '--simulate', '--no-vsync', '--output', ('"' + $run + '"'))
if ($Warp) { $arguments += '--warp' }
Get-CimInstance Win32_VideoController | Select-Object Name,DriverVersion,PNPDeviceID | ConvertTo-Json | Set-Content (Join-Path $run 'display-adapters.json')
Get-CimInstance Win32_OperatingSystem | Select-Object Caption,Version,BuildNumber | ConvertTo-Json | Set-Content (Join-Path $run 'os.json')
Get-CimInstance Win32_Processor | Select-Object Name,NumberOfLogicalProcessors | ConvertTo-Json | Set-Content (Join-Path $run 'cpu.json')
Get-FileHash $exePath -Algorithm SHA256 | ConvertTo-Json | Set-Content (Join-Path $run 'build.json')
$p = Start-Process -FilePath $exePath -ArgumentList $arguments -PassThru
if (-not $p.WaitForExit(($Seconds + $Warmup + 120) * 1000)) { $p.Kill(); throw "Benchmark timeout: $run" }
$p.Refresh()
if ($p.ExitCode -ne 0) { throw "Application failed ($($p.ExitCode)); inspect %LOCALAPPDATA%\ApexVisualization\apex.log; output: $run" }
$log = Join-Path $env:LOCALAPPDATA 'ApexVisualizationpex.log'
if (Test-Path $log) { Copy-Item $log (Join-Path $run 'application.log') }
$report = Get-Content (Join-Path $run 'report.json') -Raw | ConvertFrom-Json
if ($report.outcome -ne 'completed' -or $report.samples -eq 0) { throw "Run incomplete: $run" }
$report | ConvertTo-Json -Depth 8
Write-Host "Measured report and per-frame CSV: $run"
