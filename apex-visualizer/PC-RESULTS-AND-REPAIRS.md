# PC result analysis: evidence still needed, source defects repaired

## What is actually known from your PC

Only the screenshot `FALLBACK TO CPU (verified); CUDA EP unavailable or failed` was supplied. No PC `compute-report.json`, process trace or application log is attached or present in the repository. Existing reports under `validation/` are shipped **build-host/CI evidence**, not your RTX measurements.

| PC check | Evidence / conclusion |
|---|---|
| RTX 3070 detection | Reported by you; detection is not compute execution |
| CPU ONNX inference | Application status reports verified CPU fallback; raw numerical/timing record not supplied |
| CUDA vector-add | Unknown: its individual result/message was not supplied |
| ONNX CUDA EP inference | **Not verified**. The screenshot does not distinguish failed execution, unavailable provider/dependency or a blocked/skipped test |
| CPU/GPU latency and display-FPS comparison | Incomplete until both measured providers pass |
| Sustained RTX execution | No results supplied |
| CUDA/driver memory-leak analysis | No results supplied; memory counters alone would not establish leak freedom |
| Other module/UI/driver-reset tests on your PC | Not recorded in the supplied screenshot; cannot enumerate individual failures without the actual records |

These are evidence limits, not fabricated PC test failures. I cannot declare the exact PC root cause or every failed/skipped case from the single status line.

## Capture the actual failing run first

In the extracted updated application directory, with the old failing run still available:

```powershell
powershell -NoProfile -File .\tools\collect-compute-diagnostics.ps1
```

Attach the ZIP path printed by the script. It selects the newest report from actual local run roots (`%LOCALAPPDATA%\ApexVisualization\compute-results` or this app's `compute-results`), **excluding bundled `validation/` reports**. To choose a particular run pass `-ResultDirectory 'C:\path\to\your\actual\run'`. It copies that run's report/trace/profile, app log and hardware/build metadata; it does not include the entity JSON or upload anything automatically. User PC reports are not automatically published to GitHub by the application or collector.

You can also use **System → Copy compute diagnostics** and paste the detailed messages. The updated status reports exact reason codes and distinguishes `failed`, `unavailable`, `skipped`, `cancelled` and `not_run`. Loadable DLLs are not marked successful GPU execution.

## Concrete defects found in the source

1. **Incorrect CUDA EP initialization order.** Earlier code loaded `onnxruntime_providers_shared.dll` and `onnxruntime_providers_cuda.dll` directly, before ONNX Runtime set its provider host. An actual Windows loader test with all pinned CUDA/cuDNN files present reproduced **Win32 1114**, DLL initialization failure. ONNX Runtime's upstream bridge explicitly calls `Provider_SetHost` before loading the provider. The fix preloads only NVIDIA runtime dependencies and lets the public ORT API initialize its provider bridge. The public-API sequence fixes that ordering issue; a later driver-less CI probe still reports 1114 even with correct host initialization. Therefore 1114 alone does not establish a root cause, and this is not a claim about your PC's particular error.
2. **Dependency search excluded normal installed-library locations.** `LOAD_LIBRARY_SEARCH_DEFAULT_DIRS` does not search the ordinary process `PATH`. Dependencies outside the app folder could be missed despite being installed. The fix discovers compatible files from `runtime/gpu`, explicit `APEX_CUDA_DLL_DIRS`, `CUDA_PATH/bin`, `CUDNN_PATH/bin` and `PATH`, then loads selected x64 DLLs by full path with scoped directory registrations. Different already-loaded modules are reported as conflicts.
3. **Name-only CUDA matching could reject the correct adapter.** The fix obtains actual CUDA device handles and matches DXGI/CUDA LUIDs on Windows. Unique exact-name matching remains a fallback only when LUID information is unavailable. A standalone-kernel failure after context/identity validation no longer automatically hides the independent ONNX test; required-CUDA command success still requires both measured tests to pass.
4. **GPU installer only extracted ORT, leaving CUDA libraries unresolved.** An optional pinned self-contained NVIDIA runtime installation now supplies compatible CUDA 12/cuDNN 9 DLLs without installing Python, a compiler toolkit or modifying system PATH. Hashes and x64 PE headers are checked, files are prepared before replacement and an existing GPU runtime is backed up. Arbitrary dependency directories cannot overwrite the pinned ONNX core. A running app must be closed before replacement.
5. **Generic status concealed failure versus skip.** Reports now retain loaded ORT path, compiled provider list, driver API version, dependency filenames/paths/load errors, per-test reason codes, blockers and profiling paths. ORT warning/error logging goes into the application log. Reports include a test inventory. No provider availability list or DLL load is treated as inference proof.
6. **Reciprocal latency could be mistaken for throughput.** Existing latency-equivalent figures remain labeled; reports additionally record actual validated-loop duration and iteration rate, which include reference-check overhead. This improves measurement interpretation; no RTX performance increase is asserted without PC results.

## Dependency repair and strict verification

Close the application, then run:

```powershell
powershell -NoProfile -File .\tools\setup-gpu-runtime.ps1 -InstallDependencies
powershell -NoProfile -File .\tools\compute-benchmark.ps1 -RequireCuda
```

The first command downloads the official pinned Microsoft GPU runtime and approximately 1.41 GiB of NVIDIA runtime wheels into a local cache, extracts DLLs/licenses into the app's GPU runtime folder, and keeps a previous installation backup. No Python installation is required. A compatible NVIDIA driver and Microsoft VC++ x64 runtime remain necessary. Download, hash, architecture or loader checks alone are **not** GPU execution verification.

The second command cannot pass through CPU fallback. It requires actual vector-add readback validation **and** numerically correct ONNX CUDA inference with CPU EP fallback disabled and an actual CUDA compute node in the profile. If it fails, collect and attach the resulting run ZIP. A CUDA provider can be compiled into a runtime yet fail initialization; the new report records that distinction.

For existing compatible local libraries, omit `-InstallDependencies` and supply `-DependencyDirs` or supported runtime environment variables. `-ExtractOnly` intentionally skips dependency completeness checks and is labeled extraction-only. Pinned packages and hashes are in `tools/cuda-dependencies.json` and `COMPUTE-DEPENDENCIES.json`.

## Analysis tool and physical tests

`tools/analyze-compute-report.py` audits a selected report without executing it. It preserves original states, identifies older blocked/unattempted GPU results as skipped, rejects inconsistent passed flags with no measured calls, and does not mistake WARP for RTX evidence. A reported requested duration alone is not treated as an independently verified sustained run.

```powershell
python .\tools\analyze-compute-report.py 'C:\actual-run\compute-report.json'
# After both actual tests pass, 10 minutes per available phase:
powershell -NoProfile -File .\tools\compute-benchmark.ps1 -RequireCuda -Seconds 600
```

Actual RTX kernel/inference performance, long-duration stability, native GPU leak diagnosis, counter accuracy against Windows tools and driver reset behavior remain physical-PC tests. Existing fatal DX11 device-removal behavior still requires restart. UI, settings, local entity inputs and the renderer are preserved.

See **RUNTIME-REPAIR-VALIDATION.md** for the checks actually performed on the build host. Its data must not be substituted for your PC results.
