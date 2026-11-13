# Phase 2 performance report

## Build and evidence

- Native executable source commit: `b7af9cfff79bd8e363e24fe67007ec9c7c335ea8`. [Successful Windows build, all six module suites, two smoke tests and timed benchmark](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43362563447).
- Compiler: Visual Studio 2022, x64 Release, static runtime. Shipped EXE SHA256: `45947c027c0f6f106b599bb7bdceb50cf42bc5075cdcdbd1a4c2f191081d8913`.
- Actual OS: Microsoft Windows Server 2022 Datacenter / 10.0.20348; CPU: AMD EPYC 9V45 96-Core Processor, 4 logical processors exposed to the VM.
- Tested adapter: **Microsoft Basic Render Driver (WARP software renderer)**, DirectX feature level 11.0. These are **not RTX 3070 results** and are not Windows 10/11 desktop hardware acceptance results.
- Local six-suite Linux C++17 tests passed. All six also passed under AddressSanitizer, LeakSanitizer (detect_leaks=1) and UndefinedBehaviorSanitizer. Windows cross-compilation/linking passed. This does not instrument native Windows driver allocations.
- Corrected two validation-tool issues found during real runs: an open CSV handle blocking Windows test cleanup, and a malformed script log path. The successful final gate was run after those fixes.

The application log includes deliberate invalid-config/malformed/missing-file cases from negative module tests; those warnings are expected test evidence, followed by recovery.

Raw data: [report.json](validation/windows-warp/report.json), [per-frame CSV](validation/windows-warp/frames.csv), [CPU](validation/windows-warp/cpu.json), [OS](validation/windows-warp/os.json), [adapter/driver](validation/windows-warp/display-adapters.json), [build hash](validation/windows-warp/build.json), [application log](validation/windows-warp/application.log). These files are copied from the successful native Windows artifact, not generated benchmark claims.

## Measured workload and results

Explicit simulation, 64 entities, default visual settings, VSync off, requested cap 144 FPS, flip-discard presentation/one-frame queue. Initial measured client area: 1028 × 749. The runner constrains window size; use that measured client area, not the requested outer window dimensions, when comparing workloads. Warmup: 5 seconds. Measurement: 120 seconds. Outcome: **completed**, 16,173 measured frame intervals.

| Metric | Measured value |
|---|---:|
| Mean FPS (1000 / mean interval) | 134.777 |
| Mean frame interval | 7.420 ms |
| Approximate p95 / p99 | 7.700 / 8.600 ms |
| Minimum / maximum interval | 6.978 / 12.746 ms |
| CPU app work before Present | 0.284 (min 0.187, max 1.750) ms |
| Process CPU, all logical cores normalized | 30.278 (min 0.000, max 72.475) % |
| DX11 async draw duration on **WARP** | 6.781 (min 6.002, max 12.105) ms |
| NVIDIA device utilization | Unavailable |
| NVIDIA device VRAM | Unavailable |
| DXGI process-local VRAM | 0.000 (min 0.000, max 0.000) MiB |
| Working set | 30.905 (min 25.961, max 31.750) MiB |
| Private memory | 13.280 (min 12.000, max 14.680) MiB |
| Simulation update/poll work | 0.005 (min 0.003, max 0.265) ms |
| Working-set last minus first | -5.258 MiB |

Percentiles use 0.1ms bins; intervals >=1000ms: 0. Optional counters hold latest values between updates; repeated observations are not independent samples. CUDA and NVML were unavailable on this runner, and initialization continued successfully. DXGI's reported zero local VRAM on WARP is not an RTX memory measurement. The benchmark's CPU usage includes software rendering/driver worker threads; the small main-loop work measurement excludes Present, pacing and asynchronous driver work.

No crash occurred during this bounded run. Memory ranges and endpoint delta do not prove the absence of leaks. No baseline-equivalent before/after benchmark exists, so no percentage optimization gain is asserted. GPU draw time is not GPU utilization or end-to-end latency. This test used simulation; the JSON worker is covered separately by module tests and the 60-frame native asynchronous smoke test.

## Audit and implementation summary

The selected renderer remains DirectX 11. Added optional trusted-path NVML/CUDA discovery, real hardware dashboard counters, sampled process/DXGI memory, provider timing, high-resolution per-process frame pacing, reusable bounded benchmark reporting and reproducible PowerShell capture. Hardware/API failure disables optional metrics; fatal render failures log HRESULTs, unwind resource owners and exit nonzero. The original UI, settings schema, async data worker, visuals, JSON path clipboard behavior and static-runtime packaging are preserved. No new runtime SDK dependency was added; standard Windows SDK headers/libraries and optional installed NVIDIA driver DLLs suffice.

The renderer already had low queue depth, asynchronous non-flushing GPU queries and clipped/bounded geometry. Phase 2 preserves those working optimizations and avoids forcing GPU saturation or adding a CUDA workload that the app does not need. See [HARDWARE-VALIDATION.md](HARDWARE-VALIDATION.md) for modules, measurement definitions and exact commands.

## Remaining issues and required PC checks

1. **RTX 3070 8GB not available here.** Actual hardware FPS, utilization, dedicated/process VRAM, CUDA device/driver matching and driver-specific behavior remain unverified. No CUDA kernel or inference model exists; availability detection is not a compute-performance test.
2. **Long-duration soak and native leak diagnostics remain.** The included script supports a 30-minute measurement (`-Seconds 1800`), and longer up to 24 hours. The completed 120-second WARP run cannot establish prolonged physical-GPU stability.
3. **Device removal requires restart.** Errors exit cleanly with logged context; automatic device recreation and actual physical reset/hotplug recovery are not implemented or tested.
4. **Physical UI/display checks remain.** Module tests cover config persistence/rejection, menu draw generation, visuals, providers and logs. Every real mouse/keyboard control, clipboard system interaction, multiple monitors/DPI, resize/minimize/restore and Windows 10/11 installation needs final manual acceptance on your PC.
5. **NVIDIA counters can be unavailable.** Missing NVML/API support or duplicate adapter names disable affected counters explicitly. This is an availability limitation, not a successful zero-usage measurement.
6. **External nonsimulated data requires your authorized producer.** No live Apex dataset, external game connection, memory reader or anti-cheat interaction is included. Hardware benchmarking runs entirely with explicit test simulation.

Start with `powershell -NoProfile -File .\tools\benchmark.ps1` after extracting the Windows ZIP; the measured report and CSV are written to a unique directory under `benchmark-results`.
