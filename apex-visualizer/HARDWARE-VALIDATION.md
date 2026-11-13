# Phase 2 — hardware validation and performance audit

This is a standalone DirectX visualization app using local simulation or an explicitly selected JSON file. No game process access or external game integration is present. No RTX 3070 performance result is asserted without a measurement from that machine.

## Implemented changes

- Automatic DXGI hardware adapter discovery, highest dedicated VRAM selection, actual selected adapter name, dedicated capacity and successfully initialized DirectX feature level. Hardware device creation failure falls back to WARP. Total initialization failure, resize failure or device removal is logged and exits with a nonzero code after RAII cleanup.
- Optional NVIDIA NVML device-wide GPU utilization and total/used VRAM, sampled at 2Hz. Only a uniquely name-matched selected adapter is accepted; missing APIs, ambiguous matches and driver failures disable the relevant counters rather than reporting invented zero usage. No SDK installation or NVML redistributable is required.
- Optional CUDA driver initialization, device enumeration, selected-adapter match, driver API version and compute-capability detection. This verifies driver/device API availability only. The app renders with DirectX; it has no CUDA kernels, inference model, TensorRT, DirectML or model initialization path. CUDA workload execution and acceleration speedups are not claimed.
- DXGI per-process local VRAM usage and budget, distinct from device-wide NVML usage/capacity. Actual process working set/private memory, process CPU, paced frame interval, CPU frame work and asynchronous GPU draw duration.
- Live System page preserves the original sidebar/settings while adding available counters and explicit unavailable/warmup labels. The page scrolls within the existing responsive menu. Source processing measures provider poll/read/parse work; file delivery measures worker-start to UI handoff, **not producer-to-display or input-to-photon latency**.
- Per-process high-resolution waitable-timer frame pacing, with standard-timer and sleep fallbacks. No global timer resolution, power profile, registry or NVIDIA Control Panel changes. Existing flip-discard/two-buffer presentation, maximum one-frame queue, asynchronous timestamp queries, bounded file worker, clipped/bounded drawing and minimized/occluded throttling remain.
- Portable benchmark module, streamed CSV, bounded percentile histogram, counter summaries, null/blank unavailable data, interrupted/error outcomes and metadata. Benchmark mode starts from deterministic defaults and does not load/overwrite the saved configuration. First-frame interval is excluded. A minimized/occluded benchmark aborts rather than silently reporting a normal run.
- Fixed stale latency values after file-path changes and a Windows-only open-file cleanup problem in the new benchmark test. Config version 1, clipboard/path controls, JSON schema and existing visuals are preserved. Corrected UI text to distinguish selected file data from generated simulation.

## Reproduce on the RTX 3070 PC

Extract the Windows ZIP fully and run `apex_visualizer.exe`. Select **System** to inspect hardware and counter status. CUDA Toolkit installation is unnecessary; a working NVIDIA driver supplies the optional driver APIs. Missing NVML counters do not prevent rendering. Low utilization is expected for this lightweight renderer and is not itself a defect.

From PowerShell in the extracted application directory:

```powershell
# 5 seconds warmup + 120 seconds of measurement, 64 simulated entities, 144 FPS cap
powershell -NoProfile -File .\tools\benchmark.ps1
# Same workload at 360 FPS; compare frame time and CPU/GPU work, not GPU percentage alone
powershell -NoProfile -File .\tools\benchmark.ps1 -Cap 360
# Extended stability / memory-trend run (30 minutes of actual measurement)
powershell -NoProfile -File .\tools\benchmark.ps1 -Seconds 1800
```

The script creates a unique directory beneath `benchmark-results` containing `report.json`, `frames.csv`, adapter/driver/OS/CPU metadata, executable SHA256 and the application log. Run the same command three times under the same resolution, display scaling, power/thermal state and driver version, with the benchmark window visible and untouched. Close unrelated GPU workloads when comparing results. Device-wide counters include other applications.

For a source build, supply `-Exe .\build\Release\apex_visualizer.exe`. No execution-policy change is made by this project. If local PowerShell policy prevents scripts, use the application directly:

```powershell
.\apex_visualizer.exe --benchmark 120 --warmup 5 --simulate --entities 64 --cap 144 --no-vsync --output .\my-benchmark
```

Use a fresh output directory each direct run; direct invocation overwrites `frames.csv` and `report.json` at the selected location. JSON-provider performance can be measured with `--json "C:\My Tool\apex_entities.json"` instead of `--simulate`; label that as a different workload. The supplied writer emits only simulated test data. Rendering settings and source mode in the report describe the initial benchmark configuration.

## Measurement interpretation

`frame_ms` is measured wall-clock start-to-start time, including presentation/pacing. `cpu_work_ms` measures app update/draw-submission work before Present; it is not total thread CPU time. Process CPU is normalized across all logical cores. GPU draw time is the latest nonblocking timestamp query, not GPU utilization. CSV unavailable fields are empty and JSON counter summaries are null. CPU/memory/NVML/DXGI sample at 2Hz; held values repeat between samples. Asynchronous GPU/file metrics are also held until newer results arrive. Counter observation counts are not counts of independent hardware samples.

p95/p99 use a fixed histogram with 0.1ms bins. Intervals at or above 1000ms are counted explicitly in an overflow bin; inspect raw CSV for exact values. Working-set growth alone does not establish a leak; compare private memory and VRAM trends after warmup, repeat runs, and use native allocation diagnostics before drawing that conclusion. No before/after speedup is claimed without a comparable baseline.

## Verified results

See the accompanying `PHASE2-PERFORMANCE-REPORT.md` and raw Windows CI report for measured values and build provenance. Portable module tests, native Windows build/tests, smoke rendering and timed WARP execution are separate checks. WARP is software rendering and cannot validate RTX performance, NVML, CUDA execution or physical-device VRAM behavior.

## Remaining physical acceptance checks

- Run the benchmark on the actual RTX 3070 8GB and confirm the selected adapter, feature level, NVIDIA counters and optional CUDA driver status. Missing or ambiguous NVIDIA matching is reported as unavailable.
- Complete the 30-minute or longer hardware soak, native heap/GPU memory analysis and driver-reset/hotplug tests. Automatic device recreation is not implemented: device removal exits with a logged HRESULT and requires restart. No unsaved setting is silently persisted during failure.
- Manually exercise every checkbox/slider, clipboard actions, Save/Load/Restore, Insert/F5/F6/F10, resize/minimize/restore, mixed-DPI displays and window sizes on the target Windows installation. Automated ImGui tests exercise page generation and visual output, not every physical mouse/keyboard/monitor combination.
- No inference model or additional acceleration backend exists to initialize or validate. An authorized external data producer is needed only for nonsimulated entity data; the renderer does not invent external bones, teams or visibility. RTX testing does not require a game or any live-player source.

## Scripts/modules added

`hardware.hpp/.cpp`, `frame_pacer.hpp/.cpp`, `benchmark.hpp/.cpp`, `tests/benchmark_tests.cpp`, `tools/benchmark.ps1`; updated main/renderer/menu/system/providers, CMake and native Windows CI. Optional driver DLLs are loaded from trusted system/NVIDIA installation paths. Existing static-runtime native Windows packaging and offline source dependencies remain.
