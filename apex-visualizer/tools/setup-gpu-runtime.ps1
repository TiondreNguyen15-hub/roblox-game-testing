param(
 [string]$Destination = "$PSScriptRoot\..\runtime\gpu",
 [string[]]$DependencyDirs = @(),
 [string]$Package = ''
)
$ErrorActionPreference = 'Stop'
# Official Microsoft NuGet 1.22.0. SHA512 pinned to its official NuGet catalog.
$expected = 'QctLHrK0ltiw8EZtYl10pk+/58kYwM+tQnGqa2FGUkbyV7apswmQFgNb+Cgce3Ht/eG3G0khehwW8WUCLrOQRg=='
$temp = Join-Path ([IO.Path]::GetTempPath()) ('apex-ort-' + [guid]::NewGuid())
New-Item -ItemType Directory -Path $temp | Out-Null
try {
 if (-not $Package) {
  $Package = Join-Path $temp 'runtime.nupkg'
  Invoke-WebRequest 'https://api.nuget.org/v3-flatcontainer/microsoft.ml.onnxruntime.gpu.windows/1.22.0/microsoft.ml.onnxruntime.gpu.windows.1.22.0.nupkg' -OutFile $Package
 }
 $stream = [IO.File]::OpenRead((Resolve-Path $Package).Path)
 $hasher = [Security.Cryptography.SHA512]::Create()
 try { $actual = [Convert]::ToBase64String($hasher.ComputeHash($stream)) } finally { $stream.Dispose(); $hasher.Dispose() }
 if ($actual -ne $expected) { throw 'Official GPU runtime package hash mismatch' }
 Add-Type -AssemblyName System.IO.Compression.FileSystem
 $archive = [IO.Compression.ZipFile]::OpenRead((Resolve-Path $Package).Path)
 try {
  New-Item -ItemType Directory -Path $Destination -Force | Out-Null
  foreach ($name in @('onnxruntime.dll','onnxruntime_providers_shared.dll','onnxruntime_providers_cuda.dll')) {
   $entry = $archive.GetEntry("runtimes/win-x64/native/$name")
   if (-not $entry) { throw "Missing official package file: $name" }
   [IO.Compression.ZipFileExtensions]::ExtractToFile($entry, (Join-Path ([IO.Path]::GetFullPath($Destination)) $name), $true)
  }
  foreach ($name in @('LICENSE','ThirdPartyNotices.txt')) {
   $entry = $archive.GetEntry($name)
   if ($entry) { [IO.Compression.ZipFileExtensions]::ExtractToFile($entry, (Join-Path ([IO.Path]::GetFullPath($Destination)) $name), $true) }
  }
 } finally { $archive.Dispose() }
 foreach ($dir in $DependencyDirs) {
  # Copy only DLLs from explicitly supplied CUDA 12 / cuDNN 9 bin directories.
  Get-ChildItem -LiteralPath $dir -Filter '*.dll' -File | Copy-Item -Destination $Destination -Force
 }
 Write-Host "GPU runtime installed at $Destination. CUDA 12 and cuDNN 9 runtime DLLs plus VC++ x64 runtime are required."
 Write-Host 'Restart the application, select System, and run validation. Installation alone does not establish CUDA execution.'
} finally { Remove-Item -LiteralPath $temp -Recurse -Force }
