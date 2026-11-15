# Actual-screen overlay validation

Native source `c463747805aa80ac12087553e76f027e141dbd6a`. [Complete Windows workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43747032887). Packaged native MSVC EXE SHA256 `775a2a98cd16860f143342074b59055ef4bbf7a0124877ac46e509d4608d58b7`.

| Check | Actual result / scope |
|---|---|
| Native Visual Studio 2022 x64 build | Passed; this native executable is packaged |
| Native CTest suites | 18/18 passed; menu suite now renders nine pages and checks both live and exact-frame textures |
| Portable / ASAN+UBSAN suites | 14/14 each passed; no sanitizer error in those tested paths |
| Native capture + UI | Actual H264 -> captured pixels -> DX11 preview passed; recorded text-reader UI still showed actual OCR pixels/results |
| Both detection profiles | Apex and Roblox synthetic contract pipelines passed actual preprocessing/ORT CPU/output decoding/preview integration; wrong-profile rejection remained explicit |
| Desktop overlay / tracking | Actual separate Win32 overlay and FOV circle painted without any capture/model/focus; show/hide, independent boxes, invalid-radius rejection, transparency/non-activation/click-through properties, stale clearing, retained source-stop badge/circle and 200-repaint cleanup passed. Controller selection/axes/smoothing/gates and separate settings tests passed |
| Desktop input probe | PASS actual Windows SendInput through focus/hold guard in OWN test window; observed cursor delta=1,0; button released and cursor restored; no live game, Apex acceptance, trained-model GPU or aim accuracy tested |
| Other regressions | Actual official OBS placeholder receiver/background updates; 120-second WARP stability; actual CPU/model benchmarks; model recovery; cancellation; missing runtime/dependencies passed expected contracts. CLI benchmark scene workload is retained |
| Unchanged launcher (prior regression) | [Focused launcher workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43745297183) previously checked its then-current payload SHA/manifest and native GUI startup/close, immediate-exit reporting, target handoff/stage order/failure halt and closed-app recovery; scripts are unchanged in this update. Child install/update stages were isolated stubs, not a user-PC repair |
| Unchanged updater (prior regression) | [Focused updater workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43745300804) previously checked replace/backup, spaced paths and keeping GPU/model/settings files. Existing file protection/activation behavior is unchanged; preview PNG guide assets are copied into their own folder |
| Visual inspection | Actual ImGui CPU-rendered Screen overlay page was reviewed. Preview state is disconnected/unconfigured; it is not a Windows screenshot, hardware result or gameplay accuracy test |
| User-PC checks | Real setup clicks/file pickers, platform font/DPI/multi-monitor behavior, selected private models' RTX execution, actual Apex response to Windows input and gaming performance still require the PC. No FPS gain or game-input success is asserted |

## Actual-screen display change

A dedicated Screen overlay sidebar page controls a separate transparent topmost Windows window. Show on screen clears the F8-hidden state and immediately shows the ON badge and configured FOV circle, even before a model or capture is loaded. The selected monitor supplies a default center; selecting a valid Apex window changes the circle to that client center. Detected boxes require fresh live results and have an independent visibility switch. Source stops clear boxes while retaining the explicit enabled display. The UI reports actual Win32 window visibility; this is not a promise that exclusive-fullscreen games display desktop windows. Mouse arming and input gates are unchanged.

## Preserved interface

The loader opens on Home, with actual Capture/Detection/last-model-execution status and three setup links. Workspace navigation follows Capture -> Detection -> Tracking. Manage groups Appearance, System and Settings. Existing JSON/simulated visualization remains in Advanced > Data lab. It no longer appears behind normal workspace pages; explicit renderer/compute CLI benchmarks and smoke tests retain their scene load.

Detection is the renamed Game ESP page; its underlying page enum/action routes are retained. The profile selector, local model chooser, enabled/current-frame analysis, live/exact previews, desktop boxes, monitor alignment, CUDA/threshold/cadence/save controls, model diagnostics and recorded OCR remain connected to their existing fields/actions. Tuning, diagnostics and explanations are grouped. The old manual close/return path is removed so every page uses the same shell/footer without unbalanced UI stacks.

Tracking keeps selection/activation separate from X, Y and FOV tabs. Its readiness checks, disarmed defaults, saved configuration, counters and diagnostics are preserved. Diagnostics and long help text are tucked away instead of filling the main workflow. Home navigation never arms or starts mouse output. F9 and all hold/focus/freshness checks are unchanged.

System separates Performance from Compute checks; original validation, custom-model fields, cancellation, results and copy diagnostics remain. Appearance keeps VSync/frame cap, window sizes/accent and the old scene controls. Settings keeps save/load/defaults/file locations. Existing valid saved window sizes are loaded as before; only fresh/invalid-size defaults change to 1000x740. Expand layout is an explicit action, not a silent migration.

The dark theme, sidebar groups, purple selection marker, status/step cards, compact labels, shared page headers and persistent arming/hotkey footer form the new shell. Core rendering, capture, detectors, input algorithm, settings schemas and GPU dependencies are retained. Styling changes can change render load; no cross-release performance improvement is claimed.

CPU previews: [Screen overlay](previews/screen-overlay.png), [Home](previews/home.png), [Tracking](previews/tracking.png), [Detection](previews/detection.png). [Navigation guide](GAME-ESP-README.md), [tracking guide](TRACKING.md). Raw records: [validation/screen-overlay](validation/screen-overlay). Private weights are not published; old private-model CPU checks are historical contract sanity checks, not fresh game/GPU proof.
