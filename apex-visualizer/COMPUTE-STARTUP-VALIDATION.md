# Five-minute model-test startup: actual validation

Native source `d76fabc65483f6cba0468fec21fe65aeba3b7341`; [successful native Windows run](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43546472698); EXE SHA256 `af3d56edffd5d786654e67bad5eb2bcb6a8dcc80583d484588f344129c94e3d9`.

- Visual Studio 2022 x64 Release build and 10/10 native CTest suites passed. Nine portable Linux suites and ASan/UBSan/LSan suites also passed, including model recovery and path handling.
- Removing the default model before invoking actual --gpu-soak startup recovered the original model byte-for-byte, matched its SHA256 and ran verified CPU numerical inference. The requested GPU duration remained 300 seconds. On WARP without NVIDIA hardware, required GPU execution remained unverified and exited 6; no GPU sustained pass is fabricated.
- Selecting a nonexistent custom model produced MODEL_FILE_MISSING, blocked unexecuted phases, actual error text and resolved paths. No custom file was substituted or created.
- Quoted/relative/blank paths, missing/invalid shape profiles and preserving prior verified results are checked. Recovery fixture bytes are independently compared with the original ONNX asset.
- Existing actual official OBS 33.0.1 camera placeholder -> native YUY2 -> RGBA -> DX11 preview test passed, as did MP4/AVI capture, render smoke checks, 120-second WARP rendering, actual CPU inference/custom-model tests, cancellation, missing runtime and explicit CUDA-unavailable contracts.

The supplied PC summary omitted the actual startup exception, so its exact cause remains unconfirmed. Missing assets in the earlier small patch and hidden startup diagnostics were confirmed implementation/packaging defects and are repaired here. The updated patch includes the model and CPU runtime; it keeps the existing GPU directory and settings. It also includes embedded recovery for a missing default model.

This validates supported startup recovery, not RTX 3070 GPU execution or five-minute stability on the user's PC. Retry System > Use bundled test model > 5-minute GPU model test. If another fault remains, Copy compute diagnostics now contains the actual error. [Instructions](COMPUTE-STARTUP-FIX.md).

Raw CI evidence: [validation/windows-compute-startup](validation/windows-compute-startup). GPU dependency loads and cuDNN version calls do not stand in for inference. ONNX's bundled mathematical test model does not analyze gameplay.
