> **Latest runtime repair:** [PC evidence and fixes](PC-RESULTS-AND-REPAIRS.md) · [Actual repair validation](RUNTIME-REPAIR-VALIDATION.md). Your PC report is still required for PC-specific conclusions.

# GPU compute implementation and actual test report

## Verified build and scope

Native source commit `ea78b3e532b9d8fde6b2f8df629c566b154fa5ad`. [Successful Visual Studio 2022 x64 Release build and all validation gates](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43366750203). Shipped EXE SHA256: `0f9199dbb39ac0f6a5c152a4fc771807c930a32235c21957772f0198d8b6e67d`. Bundled model SHA256: `2d08ea139dfea3fc7273eb47a58e1168dde19073ca3526c6fee62db096900c5b`. ONNX Runtime reported version **1.22.0**.

Actual test machine: **Microsoft Windows Server 2022 Datacenter 10.0.20348**, AMD EPYC 7763 64-Core Processor, 4 logical CPUs exposed. Rendering adapter: **Microsoft Basic Render Driver**, DX11 WARP software renderer. **No NVIDIA GPU is available on this host. Your detected RTX 3070 has not been remotely tested by this run.**

## What was tested

| Check | Actual result |
|---|---|
| Native VS 2022 x64 compilation | Passed |
| Seven CTest module suites | Passed: settings/config, ImGui visuals/menu, providers/JSON, logging, benchmark, compute reference/report |
| Native JSON DX11 smoke | Passed: 6 synchronous and 60 asynchronous frames |
| Rendering regression | Completed 120 seconds after 5 seconds warmup; WARP, 12,283 measured frame intervals |
| Real ONNX CPU model execution | Passed numeric reference checks for 119,603 measured Run calls, plus 10 warmups |
| Sustained CPU inference | Minimum 60 seconds, complete; rendering remained running concurrently |
| CUDA vector-add execution here | **Unavailable**: CUDA driver DLL unavailable; no passed execution claim |
| ONNX CUDA EP execution here | **Unavailable**: CUDA device match/workload not verified; CUDA EP test not started; no GPU latency or speedup result |
| Required-CUDA negative test | Expected exit 6 confirmed by CI; CPU fallback did not count as CUDA success |
| Missing inference runtime | Expected exit 6, explicit unavailable report, no falsely verified inference |
| Cancelled compute | Expected exit 6 and cancelled report; portable policy test also rejects cancellation after CPU verification |
| Official GPU runtime installer | Downloaded, SHA512 checked and DLLs extracted successfully on Windows; **not an execution test** |
| Portable sanitizer tests | All seven passed with AddressSanitizer/UndefinedBehaviorSanitizer and leak detection enabled on Linux |
| Windows cross-build | Compiled/linked; official ORT header emits MinGW SAL macro warnings, not errors |

CPU inference above uses a real ONNX Runtime session on the bundled synthetic MatMul+Relu model, not fabricated entity data or a timed sleep. CUDA workload source calls the real driver APIs and checks readback; its successful execution remains unverified here. Portable sanitizers do not instrument Windows CUDA/ONNX/driver allocations.

## Measured CPU inference and rendering

Same app, 64 simulated entities, VSync off, cap 144 FPS. Timings include ONNX Run/copy/execution/output retrieval, exclude session setup, warmup and reference comparison. WARP competes for CPU resources, so this is not an isolated CPU performance baseline and cannot be extrapolated to the RTX PC.

| Metric | Actual value |
|---|---:|
| Verified measured CPU Run calls | 119,603 |
| Mean / minimum / maximum CPU inference latency | 0.324 / 0.183 / 10.500 ms |
| Reciprocal mean Run latency (serial equivalent only) | 3088.427 calls/s |
| Observed display FPS during CPU phase | 80.214 |
| Renderer intervals observed during CPU phase | 4,816 |
| ONNX CUDA latency / CPU-to-GPU ratio | **Unavailable / null** |
| CUDA kernel event timing | **Unavailable / null** |

The reciprocal latency figure is **not** whole-loop throughput or display FPS: every output is checked outside the timed Run, adding work. CPU/GPU comparison fields are populated only if both providers actually pass. Display-FPS phase observations include setup/warmup; short phases can be too short for comparison.

## Actual process memory and counter observations

| Counter | Before | Peak observed | After cleanup + sampling delay |
|---|---:|---:|---:|
| Private RAM (MiB) | 17.223 | 28.816 | 19.273 |
| Working set (MiB) | 35.859 | 52.293 | 38.504 |
| DXGI process local VRAM (MiB) | 0.000 | 0.000 | 0.000 |
| PDH busiest matching process engine (%) | 94.507 | 96.563 | 94.295 |

PDH returned matching PID+DXGI-LUID engine counters on this **software-rendering runner**. Those values are Windows engine accounting, **not NVIDIA hardware utilization or evidence of CUDA execution**. The zero DXGI local VRAM values on WARP likewise do not represent RTX memory behavior. On the RTX, the same filter selects that actual adapter LUID. Device-wide NVML values remain separate from process counters.

Private RAM endpoint change: 2.051 MiB; working-set endpoint change: 2.645 MiB. These are observations, not proof of a leak or leak freedom. Runtime/driver caches, allocator retention and reference initialization can affect endpoints. The trace records roughly 2Hz samples; counters are held between updates, can miss brief peaks, and 4,937 main-thread observations are not that many independent hardware measurements.

No crash occurred during the completed rendering and CPU inference runs. CUDA leak freedom, GPU memory behavior and prolonged GPU stability are **not verified** by these results.

## Implementation delivered

- Actual PTX vector-add driver workload with device context/buffers/events/module cleanup, synchronized kernel events, full numerical readback and repeated/sustained mode. No NVCC/SDK needed for this workload.
- Dynamic ONNX C API runtime with a real deterministic model, independent CPU reference, CPU provider baseline and optional CUDA EP. CUDA EP uses explicit device selection, TF32 off, CPU fallback disabled, validated host output and an actual CUDA compute-node profiling event before verification. Model/session/kernel initialization and inference failures produce explicit status/logs.
- CPU fallback status distinguishes unavailable CUDA from verified acceleration. GPU ACTIVE is shown only after execution/numerical profiling proof while that phase is running; a completed test is labeled verified rather than continuously active.
- Per-process PDH busiest-engine sampling filtered to this PID and selected DXGI LUID; per-process WDDM local VRAM/budget, RAM before/peak/after, streamed trace and observed renderer FPS per phase. NVML global utilization is separately labeled.
- Background worker with cancellation, bounded aggregate statistics/streamed traces and RAII resource lifetimes. Existing UI, settings format, DX11 renderer and authorized JSON/simulation features remain.
- Version-pinned CPU runtime/model/API header, offline source build dependencies, GPU installer with pinned official package hash, benchmark scripts, numerical/report tests and native CI failure checks.

See [GPU-COMPUTE.md](GPU-COMPUTE.md) for commands, proof criteria, dependency setup and measurement definitions. The model is an untrained mathematical workload; no live game input, game process access or external model results are synthesized.

## Raw evidence

- [Actual compute report](validation/windows-compute/compute-report.json), [sampled process trace](validation/windows-compute/compute-samples.csv), [application log](validation/windows-compute/application.log)
- [Model hash](validation/windows-compute/model.json), [executable hash](validation/windows-compute/build.json), [CPU](validation/windows-compute/cpu.json), [OS](validation/windows-compute/os.json), [display adapter/driver](validation/windows-compute/display-adapters.json)
- [Required-CUDA negative report](validation/windows-compute/compute-required-negative/compute-report.json), [missing-runtime report](validation/windows-compute/compute-missing-runtime/compute-report.json), [cancelled-run report](validation/windows-compute/compute-cancelled/compute-report.json)
- [GPU installer extracted DLL hashes](validation/windows-compute/gpu-runtime-install-manifest.json), [render-only regression report](validation/windows-compute/render-regression/report.json) and [frame CSV](validation/windows-compute/render-regression/frames.csv)

Expected warnings in the application log include intentionally malformed configuration/JSON inputs from negative tests, plus unavailable CUDA on this host.

## Remaining hardware acceptance and issues

1. **Run actual RTX tests on your PC.** The implementation is present; successful CUDA kernel and CUDA EP execution, GPU timings/latency/FPS comparison and physical process-counter accuracy remain unverified here. Driver detection or installer success is not enough.
2. **ONNX GPU dependencies are optional and separate.** The ZIP includes CPU Runtime 1.22.0. Use the supplied installer plus compatible CUDA 12/cuDNN 9 DLL directories for CUDA EP; its DLL is not included in the small native ZIP. ONNX DLLs require VC++ x64 runtime; the DX11 app itself remains statically linked.
3. **Run the sustained command and repeat full tests.** `compute-benchmark.ps1 -RequireCuda -Seconds 600` runs at least ten minutes for each available phase, about thirty minutes when all three work. Compare trace trends across repeated runs and investigate native CUDA/ORT/driver allocations before asserting leak freedom. No extended RTX soak was executed here.
4. **Driver failures retain existing behavior.** Optional compute exceptions are reported in the UI/log and do not mutate the renderer. A fatal DX11 device reset/removal still requires restart. Cancellation waits for a running driver call to return; no claim is made that a hung driver can always be interrupted.
5. **Physical UI/counter checks remain.** Verify NVIDIA PDH/VRAM readings against Windows tools, driver compatibility, DPI/resize/clipboard/buttons on your PC. Two-Hz monitoring can miss fast allocations and is not per-kernel utilization. Duplicate adapter names disable CUDA selection rather than risking a different GPU.

Start with **System → Run CUDA + ONNX validation**. For a command that cannot pass through CPU fallback, after installing the GPU dependencies run:

```powershell
powershell -NoProfile -File .\tools\compute-benchmark.ps1 -RequireCuda
```
