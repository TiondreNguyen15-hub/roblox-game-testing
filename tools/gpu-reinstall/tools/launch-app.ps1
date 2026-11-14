function Start-VerifiedApexApp {
 param([Parameter(Mandatory=$true)][string]$Exe,[string[]]$Arguments=@(),[int]$TimeoutSeconds=20,[string]$ReportDirectory='')
 $target=[IO.Path]::GetFullPath($Exe)
 $started=Get-Date
 $process=$null
 try {
  $start=@{FilePath=$target;WorkingDirectory=(Split-Path -Parent $target);PassThru=$true}
  if ($Arguments.Count) { $start.ArgumentList=$Arguments }
  $process=Start-Process @start
  $timer=[Diagnostics.Stopwatch]::StartNew();$readySince=-1
  while ($timer.Elapsed.TotalSeconds -lt $TimeoutSeconds) {
   $process.Refresh()
   if ($process.HasExited) { throw ('App exited during startup: code 0x{0:X8} ({1})' -f $process.ExitCode,$process.ExitCode) }
   if ($process.MainWindowHandle -ne [IntPtr]::Zero -and $process.MainWindowTitle -like 'Apex Visualization*') {
    if ($readySince -lt 0) { $readySince=$timer.Elapsed.TotalSeconds }
    if ($timer.Elapsed.TotalSeconds-$readySince -ge 1.5) { Write-Host 'App window reopened and stayed running through the startup check.' -ForegroundColor Green;return $process }
   } else { $readySince=-1 }
   Start-Sleep -Milliseconds 100
  }
  throw "App did not expose its main window within $TimeoutSeconds seconds (PID $($process.Id))."
 } catch {
  $failure=$_.Exception.Message
  $lines=@("Launch failed: $failure","Executable: $target","Time: $(Get-Date -Format o)")
  $log=Join-Path $env:LOCALAPPDATA 'ApexVisualization/apex.log'
  $lines+=@('','Recent application log:')
  if (Test-Path -LiteralPath $log) { $lines+=@(Get-Content -LiteralPath $log -Tail 40 -ErrorAction SilentlyContinue) }
  try { $events=Get-WinEvent -FilterHashtable @{LogName='Application';Id=1000;StartTime=$started.AddSeconds(-1)} -MaxEvents 10 -ErrorAction Stop | Where-Object { $_.Message -like '*apex_visualizer.exe*' };foreach ($event in $events) { $lines+=@('','Windows crash record:',$event.Message) } } catch { }
  if (!$ReportDirectory) { $ReportDirectory=Split-Path -Parent $target }
  $report=Join-Path $ReportDirectory 'launch-failure.txt'
  try { $lines | Set-Content -LiteralPath $report -Encoding UTF8;Write-Host "Launch diagnostics saved: $report" -ForegroundColor Yellow } catch { }
  Write-Host $failure -ForegroundColor Red
  throw $failure
 }
}
