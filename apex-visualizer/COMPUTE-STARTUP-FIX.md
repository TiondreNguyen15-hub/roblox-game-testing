# Five-minute model test startup repair

The user supplied a result in which CUDA workload, CPU inference and CUDA EP were all `not_run`, runtime path was empty and the headline was generic. The actual report error was not provided, so the exact PC cause is not established. Code inspection found reproducible startup problems: small EXE patches did not include `models/compute_validation.onnx`, paths pasted with surrounding quotes/whitespace were treated literally, and startup exceptions were omitted from copied diagnostics.

## Changes

- Missing **default** validation models are recovered from an embedded original model envelope and deterministic FP32 weights. Recovered bytes must exactly match the original ONNX fixture. This is the existing mathematical test asset, not a vision model or fabricated gameplay data.
- Existing model files and explicit custom paths are never silently substituted. Missing custom files fail with MODEL_FILE_MISSING and their resolved path.
- Paths trim surrounding whitespace/Windows copy-path quotes and resolve relative paths against the app folder. Blank/whitespace-only model paths use the bundled test model. Blank input-profile paths remain optional; missing/invalid profiles report specific errors.
- Startup failures display their actual error prominently, include it and both paths in Copy compute diagnostics, and classify unexecuted phases as blocked with the reason. No GPU success is inferred from recovering a file.
- Use bundled test model clears both optional input fields. The five-minute test still requires actual CUDA EP execution, numerical checking and at least 300 measured GPU seconds.
- The update includes `models/compute_validation.onnx` and the CPU ONNX runtime. It does not replace the existing GPU runtime, OBS camera receiver, saved settings or DX11 renderer.

## Install and retry

Close the app. Extract `Apex-Visualization-Compute-Fix.zip` into the **existing folder containing apex_visualizer.exe** and replace files when prompted. Keep your installed `runtime/gpu` directory and all its DLLs; this patch does not contain GPU libraries or require reinstalling the 1.41 GiB dependency set.

Launch the updated app. In System, click **Use bundled test model**, then **5-minute GPU model test**. Stop capture during the first benchmark for a controlled measurement. CPU preparation occurs before the GPU phase. If another error remains, Copy compute diagnostics now includes the actual exception, model/profile paths, phase and runtime, so the next report is actionable without PowerShell.

Extracting into a separate new folder will not bring over the previously installed GPU runtime. In that case CPU inference can work while CUDA EP is unavailable; use the existing installation with its GPU libraries. Recovery to a protected app folder can also fail explicitly: use a writable app folder rather than changing system security settings.

## Verification

The workflow tests a missing default asset during the actual `--gpu-soak` startup path, verifies the recovered model SHA256 and real CPU numerical inference, checks a 300-second requested GPU target, and requires explicit GPU unavailability on the driver-less CI runner. It does **not** pretend a five-minute GPU test passed there. Missing custom-model and input-profile cases are separately tested. The real official OBS camera and existing rendering/inference regressions remain in the native workflow. Actual RTX 3070 sustained results still require a run on that PC.
