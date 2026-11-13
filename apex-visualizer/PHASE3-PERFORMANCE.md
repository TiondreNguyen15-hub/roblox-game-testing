# Phase 3: selected-model benchmarking and external capture

The existing DX11 renderer, purple interface, local JSON provider, persisted settings and original validation tests remain. New benchmark model paths are session-only; existing saved config format is unchanged.

## Run on your RTX 3070

Close the app before replacing the EXE. Keep your working `runtime/gpu` folder and all its companion DLLs. Open the updated EXE normally. In **System**:

1. Leave **Model path** blank for the bundled `models/compute_validation.onnx`, or paste an absolute path to your own ONNX model. The bundled model is a mathematical test, not a gameplay detector.
2. Select **Benchmark loaded model CPU / GPU** for 10 measured seconds on each provider, or **5-minute GPU model test** for 10 CPU seconds followed by at least 300 measured GPU seconds. Initialization and ten warmup calls precede timing; total time is longer.
3. Keep the window visible and avoid changing workloads while comparing runs. Cancel remains available. Rendering and capture consume GPU resources too: stop capture for a repeatable model-only comparison, then measure again with capture if needed.
4. Inspect the CPU/GPU Run latency, validated-loop inference FPS, existing GPU utilization/VRAM counters, memory trends and each phase state. **Copy compute diagnostics** includes the report path. Send `compute-report.json` and `compute-samples.csv` for analysis.

The report records actual model path, SHA256 before/after, provider/runtime, input-shape profile SHA256, warmup policy and execution criteria. A changed file fails the run. GPU success requires actual non-Memcpy CUDA profile nodes, disabled CPU fallback, finite outputs agreeing with the CPU baseline, and a final numerical check. Benchmark-mode CPU output is a comparison baseline, not independent proof of model correctness. The original bundled validation still checks an independent C++ reference.

FP32 tensor models with 1–8 inputs, rank <=8, and 1–16 FP32 outputs are supported. Inputs use deterministic signed FP32 values, not captured gameplay. Dynamic shapes require a JSON object mapping each dynamic input name to concrete dimensions, for example `{"images":[1,3,640,640]}`. Static dimensions cannot be overridden. Limits: 64 MiB per input, 256 MiB total inputs/outputs, 64 KiB shape profile. Unsupported contracts fail explicitly. External ONNX weight files are not hashed by this build; benchmark a self-contained model for reproducible identity.

Optional command from the app directory:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\model-benchmark.ps1 -Sustained -RequireCuda
```

Custom file: append `-Model "C:\Models\your-model.onnx"` and, when needed, `-InputProfile "C:\Models\input-shapes.json"`. The script captures EXE/model hashes and adapter/driver identity. GPU tests cannot pass on a CPU-only runtime. A CPU-only benchmark can complete while the overall GPU inventory remains INCOMPLETE.

## Metrics and result meanings

- **Run latency**: measured wall time of actual ONNX `Run`, including default CPU fetches/transfers. Setup, warmup and numerical validation are excluded from this metric.
- **Inference FPS**: completed measured calls / actual measured-loop seconds, including periodic output checks and loop overhead; separate from rendering FPS. CPU/GPU latency ratio is emitted only when both providers passed.
- **GPU utilization**: existing NVML adapter-wide measurement and separately labeled PDH busiest-engine measurement filtered to this process and renderer adapter. Not a claim of CUDA-only utilization. Unsupported counters remain unavailable.
- **VRAM**: existing DXGI per-process local WDDM memory and separately labeled device-wide NVML memory. WDDM counters are not exhaustive CUDA allocator instrumentation.
- **Five-minute execution PASS** requires actual >=300 GPU measured seconds, final verification and completed phase. Missing hardware/runtime, cancellation, short runs or missing evidence remain incomplete/failed.
- **Memory growth check** separately reports passed/failed/inconclusive. After 30 GPU steady-state seconds, it requires >=240 seconds and >=70% of expected 2Hz valid counter samples. It flags growth greater than max(64MiB,2% of initial usage), regression slope >8MiB/min, and rising averages across all four quarters. A one-time allocator cache increase is not called a continuous leak. No trigger means only "stable observed", never proof of leak freedom. Driver allocations and longer-runtime leaks still require PC investigation. Histories are bounded to 2400 samples.

No RTX 3070 performance improvement or five-minute stability result has been claimed. Your earlier passing short CUDA test establishes execution for that run, not these new measurements. See PHASE3-VALIDATION.md for build/test evidence.

## External gameplay capture

See [CAPTURE.md](CAPTURE.md). The new **Capture** page supports OBS Virtual Camera through Windows Media Foundation and local recordings, real frame preview, dimensions, capture FPS, read-wait time, luma and frame activity. It does not inject, read another process, automate aiming, or invent player entities. Semantic analysis requires an authorized vision model plus its preprocessing/output contract, which have not been provided.
