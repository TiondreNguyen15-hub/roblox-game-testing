> **Updated dependency repair and PC report collection:** [PC-RESULTS-AND-REPAIRS.md](PC-RESULTS-AND-REPAIRS.md). Use `setup-gpu-runtime.ps1 -InstallDependencies` for a pinned private CUDA/cuDNN stack; extraction alone does not verify inference.

# Actual GPU compute validation

This phase adds real compute and inference tests to the existing standalone DX11 application. It does not connect inference to entity data or any external application. The included model is a deterministic, untrained matrix calculation, not a game model or detector.

## Run on the RTX 3070

1. Extract the entire Windows ZIP and run `apex_visualizer.exe`.
2. Open **System → Run CUDA + ONNX validation**. The driver test needs only a working NVIDIA driver; it JIT-compiles PTX, allocates three 4 MiB device buffers, copies inputs, launches a real vector-add kernel and reads back/checks all 1,048,576 outputs. CUDA event timings and complete copy/launch/readback wall timings are separate. Detection alone never passes this test.
3. The ZIP bundles Microsoft ONNX Runtime **CPU** 1.22.0. Its DLLs require the [Microsoft Visual C++ x64 redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe). The static-runtime DX11 application still launches when the optional inference DLLs/dependencies are unavailable; the UI reports the actual failure. CUDA driver detection does not imply CUDA Toolkit/cuDNN availability.
4. To enable ONNX CUDA EP, close the app, install the official compatible **CUDA 12.x runtime libraries and cuDNN 9.x**, then use the supplied installer from PowerShell. It downloads the official Microsoft GPU NuGet package (~208 MB), verifies its pinned SHA512, and extracts only the CUDA EP/core/shared DLLs into `runtime/gpu`. It does not install system software or change your PATH/settings.

```powershell
# Download/extract Microsoft's CUDA EP runtime. CUDA/cuDNN dependencies are separate.
powershell -NoProfile -File .\tools\setup-gpu-runtime.ps1
# From PowerShell, supply the actual CUDA 12 and cuDNN 9 DLL directories on your PC:
.\tools\setup-gpu-runtime.ps1 -DependencyDirs 'C:\your-CUDA-12-bin','C:\your-cuDNN-9-bin'
```

The second command's paths must be replaced with real directories containing the installed runtime DLLs. Typical required dependencies include `cudart64_12.dll`, `cublas64_12.dll`, `cublasLt64_12.dll` and the cuDNN 9 DLL family; the official CUDA EP build can also import other CUDA runtime DLLs. Copy all DLLs from the explicitly selected runtime bin directories rather than guessing a subset. See the [ONNX Runtime CUDA EP compatibility table](https://onnxruntime.ai/docs/execution-providers/CUDA-ExecutionProvider.html#requirements). An offline official `.nupkg` can be supplied with `-Package`; its hash is still verified. No GPU success is asserted by this installer.

5. Restart and run validation again. The app prefers `runtime/gpu/onnxruntime.dll`, otherwise the bundled CPU runtime. If the CUDA EP or its dependencies fail, a verified CPU result is reported as CPU fallback. Only a real passed CUDA inference test produces a verified CUDA status. Rename/remove `runtime/gpu` to return to the bundled CPU library.

## Reproducible CPU/GPU comparison

From the extracted application directory:

```powershell
# Minimum 30 seconds per available phase, at least 200 measured iterations per phase.
# Require BOTH actual vector-add and actual ONNX CUDA EP verification.
powershell -NoProfile -File .\tools\compute-benchmark.ps1 -RequireCuda
# Extended run: 10 minutes EACH for vector-add, CPU inference and CUDA inference.
# With all backends available this takes about 30 minutes plus initialization.
powershell -NoProfile -File .\tools\compute-benchmark.ps1 -RequireCuda -Seconds 600
```

For a source build supply `-Exe .\build\Release\apex_visualizer.exe`. A direct invocation is also supported:

```powershell
.\apex_visualizer.exe --compute-test --compute-seconds 30 --compute-iterations 200 --require-cuda --output .\my-compute-run
```

Use fresh output directories for direct invocations. The PowerShell script creates unique timestamped directories and captures executable/model hashes, display adapter/driver, CPU, OS, the actual report, process trace and application log. `--compute-test` without `--require-cuda` can exit successfully when CPU inference passed and CUDA is unavailable; read the individual verification fields. With `--require-cuda`, unavailable/unverified GPU execution returns exit code **6**. Cancelled/incomplete runs also return 6 even when CPU inference passed earlier; automated cancellation can be tested with `--compute-cancel-after 1`. Numeric/API failures also fail the command. No fallback can satisfy a required CUDA test.

The CLI uses 64 simulated entities, VSync off and a 144 FPS cap without loading or saving your preferences. Keep the window visible and leave the scene, resolution, power/thermal state and other workloads unchanged across comparisons. Record three runs. Interactive tests preserve your chosen rendering/settings and therefore may use a different visual workload. The UI's sustained button requests 5 minutes per available phase; Cancel stops at an iteration boundary and does not mark an incomplete phase passed.

## Proof of execution

- **CUDA kernel:** unique selected-adapter/device-name match, actual CUDA context, PTX JIT/module/function, device allocations, host-to-device copies, `cuLaunchKernel`, event synchronization, event elapsed time, device-to-host copy and full numerical agreement. Ten warmup iterations precede measurement. Context/module/events/device memory are RAII-owned and released when the phase ends. The fixed buffers occupy 12 MiB; this is not an 8 GB capacity or Tensor Core benchmark.
- **ONNX CPU:** `CPUExecutionProvider`, a real `OrtSession::Run` through the C API, host output type/count validation and agreement with an independent C++ reference. Model: float32 MatMul(128×256, 256×256), then Relu; deterministic nonconstant inputs/weights. One intra/inter-op thread and normal graph optimization. No constant output or fake inference timing.
- **ONNX CUDA EP:** explicit device selection, TF32 disabled for FP32 correctness comparison, CPU EP fallback disabled, successful numerical output, and an actual `CUDAExecutionProvider` compute node (MatMul/Gemm/Relu, including fused forms) in the ONNX profiling file. Profiling ends after warmup, before measured runs. The CUDA EP arena limit is 512 MiB with same-as-requested growth; this does not bound every driver/cuBLAS allocation. The CUDA driver workload runs first. A verified matching device/context is required for ONNX testing; a later standalone-kernel failure is reported independently and does not hide an otherwise testable provider. Required-CUDA command success still requires both tests.
- **Status:** the UI says CUDA **ACTIVE** only after profile/numerical proof while the measured CUDA phase is running. On completion it says **VERIFIED — test completed**, because no inference remains running. A verified CPU result with unavailable/failed CUDA reports **CPU fallback**. Missing runtime/dependencies and failed tests remain distinct from passed tests. A later failed phase does not count as a successful required-GPU run.

## Timing and process monitoring

`compute-report.json` contains per-phase actual latency counts/mean/min/max, vector-add GPU event timing, CPU-to-GPU latency ratio only when both inference tests passed, observed renderer frame intervals during each phase, and before/peak/after-cleanup process memory/counter snapshots. All measured inference outputs are checked against the reference. The sampled `compute-samples.csv` records elapsed time, phase, frame interval and process GPU/VRAM/RAM observations at about 2Hz. It is streamed, not accumulated in an unbounded vector.

ONNX Run wall latency includes copies/execution/output materialization; model/session initialization, warmup and reference checking are excluded. Reciprocal mean Run latency is a **serial inference-rate equivalent**, not display FPS or pipelined throughput. Renderer FPS is computed from actual main-window frame intervals observed during the relevant phase, including phase setup/warmup. Short phases may have too few frames for a meaningful comparison. Small models can run slower on the GPU due to transfer/launch overhead; the application does not assume a speedup.

Process GPU busy uses the real Windows `GPU Engine` PDH counter filtered by **this PID and the selected DXGI adapter LUID**. It reports the busiest matching engine, not a sum that can exceed 100% across engines. It includes this process's rendering and compute work and is not compute-only utilization. Missing counters/instances are unavailable, not zero. Device-wide NVML counters remain separately labeled. Local VRAM is DXGI's **per-process WDDM local-segment usage/budget**, not NVML device-wide usage or total physical capacity. Shared/non-local memory is not included. Counter updates occur at 2Hz; very short allocations or kernels may fall between samples. Post-cleanup report capture waits at least 1.1 seconds for fresh process sampling.

## Stability and limits

The extended command repeatedly executes and checks outputs; its trace can expose rising private memory/VRAM or degrading frame time. Repeated complete validation runs also exercise context/session cleanup. Before/after/peak memory and sanitizer checks cannot prove native CUDA/ONNX/driver leak freedom. Use native allocation tools and an actual sustained RTX run to investigate a trend. GPU tests are asynchronous to the UI; no worker touches ImGui or DX11. Cancellation cannot interrupt a driver call that has hung; Windows driver watchdog behavior still applies. Existing fatal DX11 device-loss behavior logs an HRESULT, cleans up and requires restart.

The build host has no NVIDIA GPU. See **GPU-COMPUTE-REPORT.md** for what was actually executed. CUDA kernel execution, CUDA EP execution/performance, physical process GPU counters and prolonged RTX stability remain unverified until a report from your PC contains passed results. No successful CUDA workload or GPU timing is synthesized when hardware/runtime support is absent.

## Architecture / dependencies

`compute.hpp`, `compute_common.cpp`, `compute.cpp` separate the result/report/numerical reference from Windows CUDA/ONNX worker code. `hardware.cpp` adds optional PDH sampling; `renderer.cpp` exposes its actual adapter LUID. The existing renderer, entity data, settings schema and configuration controls remain. `tests/compute_tests.cpp` tests reference validation, mismatch/NaN/shape errors, aggregation and unavailable JSON output. `models/compute_validation.onnx` is generated by `tools/generate_compute_model.py` without external Python packages.

The app uses Microsoft's version-pinned C API header and dynamically loaded runtime (MIT license and third-party notices included). CUDA driver APIs are dynamically resolved; no CUDA SDK/NVCC is needed to build or execute the vector-add test. ONNX CUDA dependencies are optional and installed separately. The offline source ZIP includes the CPU runtime and model. GPU installer extraction is validated separately from GPU execution; a downloaded DLL is not evidence of acceleration.
