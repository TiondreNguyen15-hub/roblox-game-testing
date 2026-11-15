# Tracking category validation

Native runtime source `aeffec7400f5897235fa56414f81124473053f2a`. [Complete Windows workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43737291921). Packaged MSVC x64 EXE SHA256 `00fcef88c49141c48969b316afff4c6be2fc30d7fd5c96298119e34287089e30`. The native controller test includes the smoothing-response comparison.

| Check | Actual result and scope |
|---|---|
| Native VS2022 module suites | 18/18 passed, including tracking controller, native window guards, actual FOV circle painting and previous modules |
| Portable / ASAN+UBSAN suites | 14/14 each passed; includes the added fast-vs-smoothed response comparison |
| Controller behavior | FOV/nearest-candidate selection, overlap-based local track ID continuity, independent axis disable, step bounds, direction, hold/focus/live/armed/freshness gates, no target outside FOV and non-persisted arming passed |
| Additional smoothing check | [Focused native workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43738217539) compiled and ran the updated controller test. Higher smoothing reduced the initial X response without Y leakage |
| Native window guards | Invalid selection blocked before input; own windows excluded from enumeration; normal visible external window metadata queried without reading game memory |
| Actual Windows mouse probe | PASS actual Windows SendInput through focus/hold guard in OWN test window; observed cursor delta=1,0; button released and cursor restored; no live game, Apex acceptance, trained-model GPU or aim accuracy tested |
| Native overlay | Existing transparency/click-through/non-activation checks plus actual tracking-only FOV circle pixels, invalid-radius rejection, stopped-source hiding and resource cleanup passed |
| Existing features | Seven real ImGui pages; live preview independent of model completion; both ONNX profile contract fixtures; wrong-profile rejection; actual recorded Windows OCR; OBS placeholder/background capture; 120-second software WARP rendering; CPU/model/recovery/cancel/dependency checks passed their contracts |
| Launcher | [Current focused Windows validation](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43738217539) checked this native payload hash/manifest, actual GUI startup/close, early-exit diagnostics, explicit/remembered closed-app target, stage order and stop-on-failure. Child GPU/update stages were isolated stubs; no PC GPU repair is claimed |
| Updater preservation | [Current preservation test](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43736087155) passed replace/backup, spaced paths and keeping existing GPU/model/settings files; tracking guide added to installed documents |
| Apex behavior / RTX | Not tested here. Actual game acceptance of synthetic mouse input, camera response, sensitivity, OBS alignment, trained-model accuracy/RTX execution and real-game performance require the user's PC. No gain or aim accuracy numbers are fabricated |

## Implemented controls and constraints

The new Tracking category has X / horizontal, Y / vertical and FOV overlay sections. Axis enable, response, smoothing and speed limits are separate. The circle is a radius in selected-client pixels, centered on its client; a geometric box anchor is used, not a detected joint. Local IDs are overlap associations, not game entity IDs. Candidates come from the existing captured-pixel ONNX result, not memory, JSON simulation or invented player state.

Arming defaults off and is never serialized; no game window is restored from disk. Runtime output requires enabled live Apex-profile detection, a selected visible/unminimized window with matching handle/PID/title, actual foreground focus, right-button hold, a registered global F9 stop and a fresh receiver frame. Focus/hold/F9 state are rechecked immediately before normal Windows SendInput. Per-axis speed and a hard 12-count per-step limit bound output. No automatic firing or blocked-input bypass is added. F8 is a graphics visibility switch, not a stop; releasing right mouse or pressing F9 stops/disarms.

The new capturedAt timestamp starts when the receiver constructs a frame, before local image preparation/inference. The default maximum age is 250 ms. It does not measure unknown upstream OBS/display delay or prove that the captured scene matches the selected game. Slow inference, no in-FOV candidate or stale results stop output. There is no hidden-player reconstruction or movement prediction. Model class meaning and friend/foe accuracy remain unvalidated.

Existing desktop boxes are optional; the FOV can draw independently while the selected game is focused. Main-window rendering can pause in the background while the capture/controller continues. Source/model/profile changes disarm/reset; ordinary Windows input rejection disarms and is visible. Settings persist separately in tracking-settings.json, with successful load disarmed. Defaults do not constitute universal mouse/game calibration.

The desktop probe result above is recorded directly. PASS verifies a test event in an own validation window only; SKIPPED leaves that check unverified on this host. This is not proof that Apex responds to those events. Ordinary input may be ignored by a game; the app reports acceptance separately and does not claim an input bypass. GPU inference and all game outcomes still need direct PC validation. Existing private model CPU checks are historical contract sanity checks, not gameplay or GPU proof; weights/Drive links are not published.

New modules: tracking.hpp / tracking.cpp / tracking_win32.cpp; controller/guard tests and the separate desktop input probe. The existing overlay gains circle/marker drawing, menu gains Tracking, capture frames gain a monotonic receipt timestamp, and main integrates held-input gating and background updates. DX11, detector profiles, saved model paths, OBS, old OCR, other UI and GPU dependencies remain.

[Tracking instructions](TRACKING.md), [capture/model setup](GAME-ESP-README.md). Raw records: [validation/tracking](validation/tracking), including input probe output and complete native/focused/preservation logs. No success on the user's PC is asserted.
