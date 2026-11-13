# Phase 3 validation and remaining checks

Native source commit `fc65f9fb5b56498f26c524bed741639733cbe09e`. [Actual Windows workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43529520387). EXE SHA256 `1f35dca304d42e17c8dfeb848df36d1a3da5210ec62a76de14ba276f28c4982b`. Packages use this Visual Studio 2022 x64 Release executable, not the cross-compiled check binary.

| Check | Recorded outcome | Scope |
|---|---|---|
| Native VS2022 build | Passed | Windows 2022 CI, x64 Release |
| CTest | 9/9 passed | Core, ImGui visuals, five menu pages/system, JSON provider, logging, render report, independent compute reference, memory/pixel checks, real recording decode |
| Linux tests / sanitizers | 8/8 each passed | ASan/UBSan/LSan portable coverage; no native Windows/CUDA leak-freedom claim |
| Real H264 recording | Passed | 30 decoded frames, display aperture honored, pixel activity, missing-file recovery, stop/restart |
| Full application capture preview | Passed | Actual recording, worker -> real RGBA -> reusable DX11 texture -> ImGui draw command on WARP |
| DX11 render regression | Passed | Native smoke tests and 120-second WARP rendering benchmark; software adapter is not RTX performance |
| Actual CPU ONNX inference | Passed | Bundled model, independent C++ numerical reference, >=60 measured seconds |
| Selected custom model | Passed | Different SHA256, FP32 dynamic Relu model, supplied `features:[16,8]`, >=3 measured CPU seconds; real model selection is exercised |
| Missing dynamic dimensions | Passed rejection test | Exit 6 with INPUT_SHAPE_REQUIRED; no fabricated output |
| Required-CUDA / cancel / missing runtime | Passed their negative contracts | Unverified/cancelled/unavailable execution cannot pass required GPU validation |
| CUDA/cuDNN dependency checks | Passed scoped checks | Twelve actual NVIDIA DLL loads and actual first-use cudnnGetVersion; missing companion rejected before dispatcher use |
| ONNX CUDA inference on CI | Not verified | CI has no usable NVIDIA CUDA device/driver; provider load may be explicitly hardware-blocked |
| RTX 3070 CPU/GPU comparison | Pending PC run | No Phase 3 GPU timing or speedup is asserted |
| Five-minute GPU test / memory trend | Pending PC run | Test is implemented; no sustained GPU pass or leak-free result fabricated |
| OBS Virtual Camera / captured Apex gameplay | Pending PC run | Local recording integration is verified; actual OBS device/source operation is separate |
| Semantic gameplay analysis | External model required | Current bundled mathematical model cannot identify gameplay objects |

The first native capture regression decoded all 30 frames but failed its expected dimensions because decoder display cropping was not applied. The implementation now honors the Media Foundation display aperture; native recording and full preview tests pass. Earlier partial CI attempts were superseded while adding full integration coverage. The workflow linked above is the final native validation record.

The user previously supplied a successful short RTX 3070 CUDA and ONNX CUDA EP result. That supports that reported run only, not timing, sustained performance or memory findings for this update. The native dependency probe/Windows WARP results also do not establish GPU inference. The optional CUDA runtime is preserved and GPU fallback remains explicit.

## Changes

- Added selected ONNX model hashing, FP32 contract inspection, explicit dynamic-shape profiles and CPU-reference/GPU numerical comparisons. Actual CUDA compute-node profiling and disabled CPU EP fallback remain mandatory.
- Added CPU/GPU benchmark controls and a distinct >=300-second GPU test. CPU preparation uses 10 measured seconds rather than forcing a five-minute CPU phase. Original validation controls remain.
- Added live actual inference FPS, Run latency, duration/progress and separate GPU execution, sustained execution and memory-check verdicts. Existing device/process GPU counters remain clearly labeled; their raw samples are retained in CSV.
- Added bounded sustained-memory trend analysis with minimum coverage, cache-step rejection and explicit inconclusive states. Counter trends do not prove leak freedom.
- Added OBS Virtual Camera / local-file Media Foundation source, latest-frame delivery, reusable DX11 preview texture, real luma/activity statistics and source-error recovery. No external application hook or process-memory access was introduced.
- Preserved UI theme, settings format, DX11 present/pacing, JSON entity provider and earlier modules. New model/capture paths are session controls.

New modules: `include/apex/performance.hpp`, `src/performance.cpp`, `src/model_benchmark.inc`, `include/apex/capture.hpp`, `src/capture.cpp`; new CLI helper `tools/model-benchmark.ps1`; new test/fixture modules and extended Windows workflow. Existing compute/menu/main/report modules were extended rather than replaced.

## Reproduce and supply PC evidence

Use System > Benchmark loaded model CPU / GPU, then System > 5-minute GPU model test. Keep the window visible. Select your actual supported ONNX file; blank means the bundled math model. Copy diagnostics to locate `compute-report.json` and `compute-samples.csv`. These contain actual elapsed/call metrics, model/input-profile hashes, shapes, provider evidence, memory heuristics and raw counters. Missing counters remain unavailable; capture should be stopped for the initial controlled model benchmark.

[Benchmark instructions](PHASE3-PERFORMANCE.md) · [External capture setup](CAPTURE.md). Raw CI evidence is under [validation/windows-phase3](validation/windows-phase3). The tiny test models/recording are original deterministic fixtures, not fabricated Apex live-player data.
