# Apex Visualization — cuDNN first-use crash fix

- [Small EXE patch: keep your installed GPU libraries](Apex-Visualization-CuDNN-Fix.zip)
- [Full Windows x64 package](Apex-Visualization-Windows-x64.zip)
- [Complete C++ source and build dependencies](Apex-Visualization-Source.zip)
- [PC evidence, fix and update instructions](apex-visualizer/CUDNN-CRASH-FIX.md)
- [Actual patch validation](apex-visualizer/CUDNN-FIX-VALIDATION.md)

The user's RTX 3070 passed CUDA vector-add, CPU inference and ONNX CUDA EP inference after a process-only PATH workaround. The cuDNN 9 first-use crash matched missing private-folder discovery of its companion DLLs.

This patch explicitly checks and preloads the entire pinned cuDNN DLL family by full path. Close the app, extract the small patch into the **existing folder containing apex_visualizer.exe**, and replace the executable. Keep `runtime/gpu`; no 1.41 GiB dependency reinstall is needed. Launch normally from File Explorer and rerun validation.

Existing UI, settings, DX11 renderer and authorized JSON/simulation inputs remain. GPU verification still requires actual numeric agreement and a CUDA compute-node profile, with CPU EP fallback disabled. A user run passed those checks; normal-launch confirmation of the patched binary, sustained performance and GPU leak diagnostics remain separate acceptance checks. Earlier project assets and historical evidence are preserved.
