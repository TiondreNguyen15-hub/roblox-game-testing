# Desktop overlay and preview validation

Native source `05f954fdc1b861c50772bf9d95752a777492d71c`. [Windows workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43563596867). Packaged MSVC EXE SHA256 `8f527f80c56577f57f3b10a0d37258049a9e66154e0b108aad016ae4152b40a8`.

| Check | Verified result and scope |
|---|---|
| VS2022 x64 / native suites | 15/15 passed, including actual Windows desktop overlay tests and existing modules |
| Portable + ASAN/UBSAN suites | 12/12 each passed in tested paths; this is not proof that the whole app cannot leak |
| Actual overlay window | Native test created a layered, topmost, non-activating tool window; checked color-key transparency, click-through flags/hit test, unchanged foreground focus and actual GDI box pixels using its paint path |
| Freshness and resources | Stale result removed box pixels; stopped source/disabled state hid window; 200 repaints and cleanup did not exceed the test's bounded GDI handle allowance; window destroyed |
| Monitor mapping | Fit/stretch geometry, letterboxing/pillarboxing and invalid dimensions passed. Actual multi-monitor/DPI alignment still requires PC testing |
| Background integration | Actual official OBS 33.0.1 placeholder capture -> image-dependent synthetic ONNX -> overlay continued after minimizing the native app. Both captured-frame and completed-analysis counts advanced. No real gameplay or trained-model GPU result was claimed |
| Preview lag fix | Real ImGui draw-data test checks that a live capture texture is drawn while the detector is busy and has no completed result; optional analyzed-texture selection also passed. Native exact analyzed-frame integration remains tested |
| Existing regressions | Actual H264 -> ONNX -> candidate boxes -> DX11 preview; actual recorded Windows OCR; OBS receiver; 120-second WARP render stability; CPU/custom-model inference; model recovery; cancellation and missing dependencies passed their contracts |
| Updater | Unchanged executable-replacement script from [its Windows preservation test](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43558633916); native payload SHA256 checked. GPU files/models/settings are preserved. GUI EXE selection/closing a real app were not automated |
| Combined GPU repair + update launcher | [Focused Windows validation](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43564570410) passed actual payload SHA/manifest validation, exact spaced path handoff, stage ordering and halt-on-failure using isolated stub child stages with the actual native app. It did not install GPU dependencies or run RTX compute in that test |
| Actual RTX 3070 / gameplay | No new benchmark or accuracy claim. Requires the user's PC, local trained model, actual OBS scene and borderless game. F8 while game-focused, exclusive-fullscreen limitations, capture exclusion behavior, actual mouse interaction and multi-monitor placement require manual confirmation |

## Confirmed user CPU fallback

The user supplied `CPU fallback: Installed ONNX runtime has no CUDA provider`, with the correct trained-model hash. This confirms that their selected runtime was CPU-only; it does not indicate a model-shape error or prove GPU-driver incompatibility. **Update-And-Enable-GPU.cmd** targets the single running demo, runs the existing pinned GPU dependency helper, then installs this exact native update into that captured path. No repair is claimed to have run on the PC here. Verify the trained model afterward through its actual CUDA profile status. The runtime helper itself remains unchanged and was exercised by the native workflow dependency installation/loading checks; installer success alone is not GPU inference proof.

## Preview issue and behavior

The previous Game ESP preview used only the analyzed frame. Its 150 ms submission interval alone limited preview changes to about 6.7/second, before any model time. The default preview now draws the newest captured texture independently; an optional exact analyzed-frame checkbox preserves the previous inspection view. Only that mode uploads an additional analyzed-frame GPU texture. Actual capture FPS, model processing latency, result age and UI frame time are separate diagnostics. This removes coupling; it does not demonstrate an RTX speedup or rule out CPU/GPU contention on the user's PC.

Desktop overlay boxes use the latest actual detector result and expire after one second without completion. They have capture/inference delay and cannot predict motion. Recordings remain preview-only. Overlay is opt-in each launch; F8 uses a registered Windows hotkey (no input hook), with checkbox fallback if already reserved. Rendering uses a separate click-through GDI layered window; the DX11 renderer is preserved. Capture/inference continue at a background loop cadence while overlay is enabled; hidden main-window rendering is skipped. Turning the overlay off while minimized hides it before normal background suspension.

Windows capture exclusion is requested; a rejected request is reported, not silently treated as supported. OBS scene alignment and actual exclusion must still be checked on the PC. Use a full-view Window Capture scene and borderless gameplay on the selected monitor. Arbitrary scene crops, tracking smaller game windows and in-game injection are not implemented.

The initial background-test harness failed because a preceding test unregistered the official camera. The harness now registers/unregisters its own verified camera component; the final workflow above passed. The implementation also passed native overlay flags/painting/staleness/resource tests before that harness repair.

The private supplied model previously passed ONNX checking and actual Linux CPU execution on black input (see raw private-model-cpu-check.json, no weights). Its SHA256 is `e7f89487dc8970e8f6da4e70ed56756d3287b4f3dfef97ce973ef7138be280bc`; shape FP32 [1,3,416,416] -> [1,300,6]. Metadata says class 0 = item; Enemy? is based on user description, not established friend/foe accuracy. GPU status becomes profiled only after an actual non-copy CUDA node executes. Synthetic fixture weights remain under source/tests, outside runtime models; private weights are not published.

New overlay modules: desktop_overlay.hpp / desktop_overlay.cpp, overlay_geometry.cpp, portable mapping tests and native desktop-window tests. Enemy observations gain a monotonic completion timestamp. Menu/main add opt-in overlay/monitor/aspect controls, F8, background processing, independent live preview, optional exact preview, and copied timing diagnostics. Existing settings, UI pages, renderer, GPU dependencies and model files are retained. No weapons/ammo model, protected memory access, game injection or gameplay automation is added.

Raw records: [validation/desktop-overlay](validation/desktop-overlay). [Instructions](GAME-ESP-README.md).
