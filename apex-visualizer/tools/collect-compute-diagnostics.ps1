param(
 [string]$ResultDirectory = '',
 [string]$AppDirectory = "$PSScriptRoot\..",
 [string]$OutputDirectory = "$env:LOCALAPPDATA\ApexVisualization\diagnostics"
)
$ErrorActionPreference='Stop'
$AppDirectory=[IO.Path]::GetFullPath($AppDirectory)
if (-not $ResultDirectory) {
 # Search only genuine local run roots. Never select shipped validation/CI evidence.
 $roots=@((Join-Path $env:LOCALAPPDATA 'ApexVisualization/compute-results'),(Join-Path $AppDirectory 'compute-results'))
 $reports=@(foreach ($root in $roots) { if (Test-Path $root) { Get-ChildItem $root -Recurse -Filter compute-report.json -File } })
 $latest=$reports | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
 if (-not $latest) { throw 'No local run report found. Run System > CUDA + ONNX validation, or pass -ResultDirectory for your actual run folder.' }
 $ResultDirectory=Split-Path $latest.FullName
}
$ResultDirectory=(Resolve-Path $ResultDirectory).Path
if (-not (Test-Path (Join-Path $ResultDirectory 'compute-report.json'))) { throw 'Selected directory has no compute-report.json' }
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$stamp=Get-Date -Format 'yyyyMMdd-HHmmss-fff';$stage=Join-Path $OutputDirectory ('compute-diagnostics-'+$stamp)
New-Item -ItemType Directory -Path $stage | Out-Null
foreach ($pattern in @('compute-report.json','compute-samples.csv','cuda-profile*.json','build.json','model.json','display-adapters.json','cpu.json','os.json')) {
 Get-ChildItem -LiteralPath $ResultDirectory -Filter $pattern -File | Copy-Item -Destination $stage
}
$log=Join-Path $env:LOCALAPPDATA 'ApexVisualization/apex.log'
if (Test-Path $log) { Copy-Item $log (Join-Path $stage 'apex.log') }
@{collected_utc=[DateTime]::UtcNow.ToString('o');source_directory=$ResultDirectory;app_directory=$AppDirectory;bundled_ci_reports_included=$false} | ConvertTo-Json | Set-Content (Join-Path $stage 'collection.json')
Get-CimInstance Win32_VideoController | Select-Object Name,DriverVersion | ConvertTo-Json | Set-Content (Join-Path $stage 'current-display-adapters.json')
if (Test-Path (Join-Path $AppDirectory 'BUILD-INFO.json')) { Copy-Item (Join-Path $AppDirectory 'BUILD-INFO.json') (Join-Path $stage 'application-build-info.json') }
$zip=$stage+'.zip';Compress-Archive -LiteralPath $stage -DestinationPath $zip
Write-Host "Attach this ZIP for analysis: $zip"
Write-Host 'It contains selected test reports and application logs/paths, not your entity JSON file. Nothing was uploaded automatically.'
