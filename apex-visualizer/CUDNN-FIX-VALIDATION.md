# cuDNN crash-fix validation

Native source commit `7177fef1fcb416ecb07d5a0fdf137a8cf871a45d`. [Windows validation run](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43514496776). EXE SHA256: `fe627810a20be7867f56742ac1af4cd6f7f154f8b149c27aa83aeea89fa36c74`.

- Visual Studio 2022 x64 Release compilation and seven CTest suites passed. Portable Linux suites and Windows cross-compilation also passed.
- Real DX11 smoke/render regression, actual CPU ONNX inference, required-CUDA, cancellation and missing-runtime checks passed their respective contracts on CI. CUDA numerical inference is not asserted on the software-rendering runner.
- All **12** core CUDA/cuDNN DLLs, including the full pinned cuDNN family, were loaded by absolute path. The real first-use **`cudnnGetVersion`** dispatcher call returned successfully. This directly exercises the discovery path implicated in the PC crash, without claiming GPU computation.
- The negative regression removed `cudnn_graph64_9.dll`; the loader rejected it cleanly with exit 1 and an explicit missing-companion message before dispatcher use, rather than native fail-fast.
- ONNX provider probe status: **blocked without a usable driver**. No module-load success is counted as numerical CUDA inference.

The user separately reported all three actual RTX 3070 validation tests passed after the process-only PATH workaround. That is evidence of a successful user GPU run; the patched EXE's normal-launch behavior still needs confirmation on that PC. Sustained performance, latency/FPS comparison values and leak freedom have not been established.

See [CUDNN-CRASH-FIX.md](CUDNN-CRASH-FIX.md) for the fault-module evidence, matching loader defect, upstream references and update instructions.

Raw records: [dispatcher/provider probe](validation/windows-cudnn-fix/dependency-loader-test.txt), [missing-companion result](validation/windows-cudnn-fix/missing-cudnn-component.txt), [installed DLL hashes](validation/windows-cudnn-fix/gpu-runtime-install-manifest.json), [compute report](validation/windows-cudnn-fix/compute-report.json), [render regression](validation/windows-cudnn-fix/render-regression/report.json).
