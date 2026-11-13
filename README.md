# Apex Visualization — standalone Windows data and compute lab

- [Download updated native Windows x64 build](Apex-Visualization-Windows-x64.zip)
- [Download complete C++ source and offline dependencies](Apex-Visualization-Source.zip)
- [Actual compute test report and remaining RTX checks](apex-visualizer/GPU-COMPUTE-REPORT.md)
- [CUDA/ONNX GPU setup, benchmark and sustained-test commands](apex-visualizer/GPU-COMPUTE.md)
- [Build instructions](apex-visualizer/README.md)

Extract the Windows ZIP completely and run `apex_visualizer.exe`. Open **System → Run CUDA + ONNX validation** for actual vector-add and inference checks. CUDA detection alone never marks execution passed.

The app includes a real CUDA driver/PTX workload, deterministic ONNX model, CPU/CUDA EP comparison, process GPU/VRAM/RAM monitoring, and a streamed sustained-test trace. The ZIP bundles CPU ONNX Runtime; the GPU installer supplies optional CUDA EP files, with compatible CUDA 12/cuDNN 9 and VC++ x64 dependencies needed separately. No rebuild is required to use the optional GPU runtime.

Native Windows validation passed seven module suites, both DX11 smoke tests, 120 seconds of WARP rendering, 60 seconds of actual CPU ONNX inference, GPU-package download/hash/extraction, and missing-CUDA/runtime/cancellation checks. **RTX 3070 kernel execution, CUDA EP execution, performance and GPU leak freedom remain unverified by the build host.** Read the report for raw measured data and commands that require real GPU success.

Existing visual settings, authorized JSON input and the DX11 renderer are preserved. The model is an untrained mathematical workload, independent of entity data. Earlier Roblox files remain in this repository.
