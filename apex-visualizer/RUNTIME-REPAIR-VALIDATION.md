> **Latest cuDNN crash fix:** [PC outcome and patch](CUDNN-CRASH-FIX.md) · [Actual validation](CUDNN-FIX-VALIDATION.md). A later user run passed CUDA inference with the process-only PATH workaround; earlier missing-data statements below are historical.

# Runtime repair validation — build host, not your PC

[Passed native Windows run](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43372129649); native source commit `e24ca35755a0a82315e09ab2a10725ffcb2ebb17`. Shipped EXE SHA256 `a3020ebaccd72ba2278d4a5c56e5d220558149df4bf609b9af8a210452a5dfff`.

## Actual PC evidence still missing

The attached screenshot reports CPU fallback only. No actual PC report/log was supplied. It is not possible to enumerate every PC failed/skipped test, establish its exact root cause, measure RTX acceleration or assert GPU leak freedom from that status line. [PC-RESULTS-AND-REPAIRS.md](PC-RESULTS-AND-REPAIRS.md) separates known evidence, source defects and the file collector command.

No GPU success is claimed from a module load, installed driver, provider list or synthetic metric. PC logs are not automatically uploaded or published.

## Verified repairs and tests

- Visual Studio 2022 x64 Release compilation and all seven CTest suites passed. Linux portable suites and all seven ASan/UBSan/leak-detection suites passed; six report-auditor unit tests passed. Windows cross-compilation/linking passed.
- Real DX11 WARP synchronous/async JSON smoke tests passed, followed by 120 seconds of render regression and 60 seconds of real CPU ONNX inference. CPU output reference checks passed for 121,590 measured calls.
- The official ONNX GPU package and pinned NVIDIA Windows x64 runtime wheel downloads were hash-checked and extracted. The actual Windows dependency test loaded all five required NVIDIA DLLs. CUDA provider loading was then attempted **through ONNX Runtime's initialized provider bridge**. Provider initialization remained blocked with error 1114 on this driver-less host; it was explicitly recorded as blocked (exit 77), not passed. No GPU inference was verified.
- An earlier repair run [failed with Win32 1114](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43370100488) in a direct provider-loading probe. A subsequent correctly initialized bridge probe also returned 1114 without an NVIDIA driver, so that error alone does not prove a cause on your PC. That run skipped subsequent required-CUDA, cancellation and missing-runtime checks after the failed gate. Those are CI failures/skips, **not your PC results**. The current run above passed the software/negative gates and explicitly retained the driver-dependent provider probe as blocked. It did not turn that probe into GPU success.
- Required-CUDA, cancellation and missing-runtime negative checks passed with expected nonzero exits. CPU fallback cannot satisfy required GPU validation. The updated app exports precise states/reason codes, loaded runtime/providers, dependency diagnostics and actual validated-loop timing, while preserving existing UI/settings/renderer/data features.

The source-order fix follows Microsoft's v1.22.0 [ProviderSharedLibrary::Ensure / ProviderLibrary::Load](https://github.com/microsoft/onnxruntime/blob/v1.22.0/onnxruntime/core/session/provider_bridge_ort.cc), which sets the shared provider host before loading the CUDA DLL. NVIDIA dependencies may be preloaded; ORT provider initialization is owned by its public API.

## Every recorded compute check, including skipped/incomplete checks

The adapter is **Microsoft Basic Render Driver**. This runner has no usable NVIDIA CUDA driver/device. Read these as build-host limitations, not failed RTX tests.

| Check | Recorded state | Reason/evidence |
|---|---|---|
| cuda_workload | unavailable | CUDA_DRIVER_NOT_LOADABLE |
| cpu_inference | passed | VERIFIED_EXECUTION |
| cuda_ep_inference | skipped | BLOCKED_BY_CUDA_DEVICE |
| cpu_gpu_comparison | skipped | Requires passed CPU and CUDA EP inference |
| sustained_cuda_inference | incomplete | Recommended minimum 600 seconds per phase; inspect actual loop duration and verification |
| native_gpu_leak_analysis | not_recorded | Memory counters alone cannot establish driver/CUDA leak freedom |

The CPU/GPU comparison is unavailable without passed CUDA inference. Long-duration GPU execution and native GPU leak analysis remain incomplete/unrecorded; memory counters do not establish leak freedom. Missing hardware is never replaced by simulated success.

## Raw evidence

- [Compute report](validation/windows-runtime-repair/compute-report.json), [process trace](validation/windows-runtime-repair/compute-samples.csv), [application log](validation/windows-runtime-repair/application.log)
- [Actual dependency/provider-bridge load result](validation/windows-runtime-repair/dependency-loader-test.txt), [installed DLL hashes](validation/windows-runtime-repair/gpu-runtime-install-manifest.json)
- [Required-CUDA negative report](validation/windows-runtime-repair/compute-required-negative/compute-report.json), [cancelled run](validation/windows-runtime-repair/compute-cancelled/compute-report.json), [missing-runtime report](validation/windows-runtime-repair/compute-missing-runtime/compute-report.json)
- [Render regression](validation/windows-runtime-repair/render-regression/report.json), [executable hash](validation/windows-runtime-repair/build.json), [hardware metadata](validation/windows-runtime-repair/display-adapters.json)

## Next actual PC checks

Close the application, run `tools/setup-gpu-runtime.ps1 -InstallDependencies`, then `tools/compute-benchmark.ps1 -RequireCuda`. After a failure, run `tools/collect-compute-diagnostics.ps1` and attach its ZIP. On successful GPU inference, run at least `-Seconds 600` per available phase and compare repeated runs/memory traces. Existing DX11 fatal device loss still requires restart, and a hung driver call cannot always be interrupted. Driver compatibility, full RTX execution, physical-counter accuracy and sustained GPU stability are unverified here.
