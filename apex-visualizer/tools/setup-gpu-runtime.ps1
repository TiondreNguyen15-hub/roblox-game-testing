param(
 [string]$Destination = "$PSScriptRoot\..\runtime\gpu",
 [string[]]$DependencyDirs = @(),
 [string]$Package = '',
 [switch]$InstallDependencies,
 [switch]$ExtractOnly,
 [string]$CacheDirectory = "$env:LOCALAPPDATA\ApexVisualization\packages"
)
$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
$appRoot=[IO.Path]::GetFullPath("$PSScriptRoot\..")
foreach ($process in @(Get-Process -Name apex_visualizer -ErrorAction SilentlyContinue)) {
 if ($process.Path -and [IO.Path]::GetFullPath($process.Path).StartsWith($appRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) { throw 'Close this application before replacing GPU runtime files, then rerun setup.' }
}
if ($InstallDependencies -and $ExtractOnly) { throw 'Choose -InstallDependencies or -ExtractOnly, not both' }
$expected = 'QctLHrK0ltiw8EZtYl10pk+/58kYwM+tQnGqa2FGUkbyV7apswmQFgNb+Cgce3Ht/eG3G0khehwW8WUCLrOQRg=='
$Destination = [IO.Path]::GetFullPath($Destination)
$parent = Split-Path $Destination
New-Item -ItemType Directory -Path $parent -Force | Out-Null
New-Item -ItemType Directory -Path $CacheDirectory -Force | Out-Null
$stage = Join-Path $parent ('gpu.stage-' + [guid]::NewGuid())
New-Item -ItemType Directory -Path $stage | Out-Null
$backup = ''
Add-Type -AssemblyName System.IO.Compression.FileSystem
function Extract-RuntimeFiles([string]$file, [string]$kind, [string]$name) {
 $archive = [IO.Compression.ZipFile]::OpenRead((Resolve-Path $file).Path)
 try {
  foreach ($entry in $archive.Entries) {
   $base = [IO.Path]::GetFileName($entry.FullName)
   $allowed = if ($kind -eq 'ort') { $entry.FullName -in @('runtimes/win-x64/native/onnxruntime.dll','runtimes/win-x64/native/onnxruntime_providers_shared.dll','runtimes/win-x64/native/onnxruntime_providers_cuda.dll') } else { $base -match '\.dll$' }
   if ($allowed) { [IO.Compression.ZipFileExtensions]::ExtractToFile($entry, (Join-Path $stage $base), $true) }
   if ($base -and $entry.FullName -match '(?i)(license|notice)' -and $base -notmatch '\.dll$') {
    $licenseDir = Join-Path $stage "licenses/$name"
    New-Item -ItemType Directory -Path $licenseDir -Force | Out-Null
    $safe = $entry.FullName -replace '[/\\:]','_'
    if ($entry.Length -gt 0) { [IO.Compression.ZipFileExtensions]::ExtractToFile($entry, (Join-Path $licenseDir $safe), $true) }
   }
  }
 } finally { $archive.Dispose() }
}
function Test-X64([string]$file) {
 $stream = [IO.File]::OpenRead($file);$reader = New-Object IO.BinaryReader($stream)
 try { if ($reader.ReadUInt16() -ne 0x5a4d) { return $false };$stream.Position=0x3c;$offset=$reader.ReadUInt32();$stream.Position=$offset;return ($reader.ReadUInt32() -eq 0x4550 -and $reader.ReadUInt16() -eq 0x8664) }
 finally { $reader.Dispose();$stream.Dispose() }
}
try {
 if (-not $Package) {
  $Package = Join-Path $CacheDirectory 'microsoft.ml.onnxruntime.gpu.windows.1.22.0.nupkg'
  if (-not (Test-Path $Package)) { Invoke-WebRequest 'https://api.nuget.org/v3-flatcontainer/microsoft.ml.onnxruntime.gpu.windows/1.22.0/microsoft.ml.onnxruntime.gpu.windows.1.22.0.nupkg' -OutFile ($Package+'.partial');Move-Item ($Package+'.partial') $Package }
 }
 $stream = [IO.File]::OpenRead((Resolve-Path $Package).Path);$hasher=[Security.Cryptography.SHA512]::Create()
 try { $actual=[Convert]::ToBase64String($hasher.ComputeHash($stream)) } finally { $stream.Dispose();$hasher.Dispose() }
 if ($actual -ne $expected) { throw 'Official GPU runtime hash mismatch; remove the cached package and retry' }
 Extract-RuntimeFiles $Package 'ort' 'onnxruntime-1.22.0'
 if ($InstallDependencies) {
  $manifest = Get-Content (Join-Path $PSScriptRoot 'cuda-dependencies.json') -Raw | ConvertFrom-Json
  $total = ($manifest.packages | Measure-Object bytes -Sum).Sum
  Write-Host ('Pinned NVIDIA runtime downloads: {0:N2} GiB. No Python/toolkit installation or system PATH changes.' -f ($total/1GB))
  foreach ($item in $manifest.packages) {
   $file = Join-Path $CacheDirectory $item.filename
   if (-not (Test-Path $file)) { Invoke-WebRequest $item.url -OutFile ($file+'.partial');Move-Item ($file+'.partial') $file }
   if ((Get-FileHash $file -Algorithm SHA256).Hash.ToLowerInvariant() -ne $item.sha256) { throw "NVIDIA package hash mismatch: $file" }
   Extract-RuntimeFiles $file 'nvidia' $item.name
  }
 } elseif (-not $ExtractOnly) {
  # Explicit locations first, then installed runtime variables/PATH. Copy only NVIDIA
  # DLLs: a dependency directory must never overwrite our version-pinned ORT core.
  $locations = @($DependencyDirs)
  if ($env:CUDA_PATH) { $locations += (Join-Path $env:CUDA_PATH 'bin') }
  if ($env:CUDNN_PATH) { $locations += $env:CUDNN_PATH; $locations += (Join-Path $env:CUDNN_PATH 'bin') }
  $locations += ($env:APEX_CUDA_DLL_DIRS -split ';');$locations += ($env:PATH -split ';')
  foreach ($dir in ($locations | Where-Object { $_ -and (Test-Path -LiteralPath $_ -PathType Container) } | Select-Object -Unique)) {
   foreach ($file in (Get-ChildItem -LiteralPath $dir -Filter '*.dll' -File)) {
    if ($file.Name -match '^(cudart64_12|cublas.*64_12|cufft64_11|cudnn.*64_9|nvJitLink_120_0|nvJitLink_?64_12|nvrtc.*64_12).*\.dll$' -and -not (Test-Path (Join-Path $stage $file.Name))) { Copy-Item -LiteralPath $file.FullName -Destination $stage }
   }
  }
 }
 $required=@('onnxruntime.dll','onnxruntime_providers_shared.dll','onnxruntime_providers_cuda.dll')
 if (-not $ExtractOnly) { $required += @('cudart64_12.dll','cublasLt64_12.dll','cublas64_12.dll','cufft64_11.dll','cudnn64_9.dll') }
 foreach ($name in $required) {
  $file = Join-Path $stage $name
  if (-not (Test-Path $file)) { throw "Missing $name. Use -InstallDependencies for the pinned self-contained NVIDIA runtime, or provide compatible DLL directories. Existing runtime preserved." }
  if (-not (Test-X64 $file)) { throw "Wrong architecture for $name; Windows x64 required. Existing runtime preserved." }
 }
 Get-ChildItem $stage -Filter '*.dll' | Get-FileHash -Algorithm SHA256 | Select-Object Algorithm,Hash,@{N='File';E={Split-Path $_.Path -Leaf}} | ConvertTo-Json | Set-Content (Join-Path $stage 'installed-files.json')
 # Files are prepared/checked before replacing the directory. Keep a backup for rollback.
 if (Test-Path $Destination) { $backup=$Destination+'.backup-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff');Move-Item -LiteralPath $Destination -Destination $backup }
 try { Move-Item -LiteralPath $stage -Destination $Destination } catch { if ($backup -and -not (Test-Path $Destination)) { Move-Item -LiteralPath $backup -Destination $Destination };throw }
 Write-Host "Runtime files prepared at $Destination. Backup: $backup"
 if ($ExtractOnly) { Write-Host 'Extraction only: CUDA dependencies and execution were NOT verified.' }
 else { Write-Host 'x64 files found; loader/driver/inference still require an actual test. Restart the app, then run compute-benchmark.ps1 -RequireCuda.' }
 Write-Host 'Microsoft VC++ x64 runtime is required: https://aka.ms/vs/17/release/vc_redist.x64.exe'
} finally { if (Test-Path $stage) { Remove-Item -LiteralPath $stage -Recurse -Force } }
