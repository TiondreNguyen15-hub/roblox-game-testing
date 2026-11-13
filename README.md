# Apex Visualization — CUDA runtime repair and diagnostics

- [Download updated Windows x64 build](Apex-Visualization-Windows-x64.zip)
- [Complete source and offline build dependencies](Apex-Visualization-Source.zip)
- [PC evidence needed, source fixes, and one-command setup](apex-visualizer/PC-RESULTS-AND-REPAIRS.md)
- [Actual build-host validation and unverified GPU checks](apex-visualizer/RUNTIME-REPAIR-VALIDATION.md)

Extract all files and run `apex_visualizer.exe`. Existing UI/settings, DX11 rendering and authorized JSON input remain.

The updated System page distinguishes failures, unavailable dependencies and skipped tests, exposes runtime/provider paths and offers **Copy compute diagnostics**. CUDA selection uses the actual adapter LUID where available. NVIDIA dependencies are loaded by explicit path; ONNX Runtime owns its CUDA provider host initialization.

To capture your real PC results, run `tools/collect-compute-diagnostics.ps1` and attach the ZIP it prints. It excludes the bundled `validation/` evidence. The current user screenshot establishes CPU fallback only; exact PC failure reasons remain unknown without that report.

For a private pinned GPU dependency installation, close the app and run `tools/setup-gpu-runtime.ps1 -InstallDependencies`; then `tools/compute-benchmark.ps1 -RequireCuda`. The installer downloads NVIDIA runtime wheels plus Microsoft's CUDA EP package and retains a backup. Installation, DLL loading and CPU fallback are never counted as passed GPU inference.

Native CPU/rendering/negative tests and the actual NVIDIA dependency DLL load check passed on Windows. ONNX CUDA provider initialization remains explicitly blocked without the NVIDIA driver on CI. **RTX kernel/inference execution, performance and GPU leak freedom remain unverified here.** Earlier Roblox assets remain preserved.
