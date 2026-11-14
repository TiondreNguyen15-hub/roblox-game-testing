# OBS compatibility update: actual validation

Native source commit `110547ab77eba226ca66879a5b905317af1283ba`. [Successful Windows validation run](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43540051211). Native EXE SHA256: `fa0cdae9c5bb7ac95c5d04b08f9cadcde91bdc946fcf71750226366dc81321cb`.

- Visual Studio 2022 x64 Release build and **all nine CTest suites passed**. Portable Linux module suites and Windows cross-compilation also passed.
- The extended Windows capture suite executed the actual **DirectShow graph**, original AVI test recording, Windows RGB32 conversion, the real Sample Grabber callback COM vtable and conversion to RGBA. It required a received frame with the expected 160x90 dimensions and nonblack real pixel statistics, then stopped and released the graph. This is a real file-decoding integration test, not a simulated OBS camera or evidence that a physical OBS source is installed on CI.
- Actual DirectShow video-input enumeration and strict OBS-name selection were exercised. On a runner without OBS, starting the camera path had to fail cleanly with DirectShow diagnostics. The existing actual H264 recording test, missing-file handling and stop/restart also passed.
- The full application recording-to-DX11 preview check passed: real MP4 frames were uploaded into the app's reusable texture and appeared in an actual ImGui image draw command. This is Media Foundation recording coverage; an actual OBS DirectShow camera needs the user's PC.
- Renderer smoke tests and the 120-second WARP benchmark passed. Actual CPU ONNX inference, selected dynamic-model benchmark, missing shape, missing runtime, required-CUDA and cancellation regression checks passed their documented contracts.
- CUDA dependency/dispatcher checks were retained. The CI machine has no usable NVIDIA CUDA driver, so **no RTX 3070 GPU inference or performance result is claimed** by this patch.

The reported failure is consistent with an API mismatch: the original button enumerated only Media Foundation; a DirectShow-only OBS registration would be missed even if OBS was running. The patched button tries DirectShow when no MF OBS camera is found and exposes actual backend errors and camera names. This has not remotely proven the cause on the user's PC or that their OBS registration is healthy.

[Update instructions](OBS-CAPTURE-FIX.md). After installing the patch, run Capture > Start OBS Virtual Camera. If it still fails, use Copy capture diagnostics and send that actual text. No model/runtime reinstall or CUDA configuration changes are needed for this capture-only patch. The source/settings/renderer remain; only capture support and diagnostics were extended.

Raw native evidence: [validation/windows-obs-fix](validation/windows-obs-fix). The test AVI is an original FFmpeg `testsrc2` recording (160x90,10fps,1 second, uncompressed BGR24); no external gameplay or fabricated player data is used.
