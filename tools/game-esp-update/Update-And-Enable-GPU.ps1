param([switch]$ValidateOnly)
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
 if ($running.Count -ne 1) { throw 'Leave exactly ONE copy of your working demo open, then run this launcher again.' }
 $target=[IO.Path]::GetFullPath($running[0].Path)
 Write-Host "Repairing GPU dependencies and updating this exact copy: $target" -ForegroundColor Cyan
 Run-Stage $gpu @('-TargetExe',('"'+$target+'"'),'-NoLaunch')
 Run-Stage $update @('-TargetExe',('"'+$target+'"'))
 Write-Host 'Finished. In Game ESP select your model, enable Prefer CUDA and analyze a captured frame.' -ForegroundColor Green
 Write-Host 'Only CUDA EP execution profiled confirms this model actually executed GPU work. Installation is not verification.'
 exit 0
} catch { Write-Host "STOPPED: $($_.Exception.Message)" -ForegroundColor Red;exit 1 }
