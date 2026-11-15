# Detection and target stability / fast response validation

Native source `b8ed322527bad9e14f8f098dcf28d8ee47b92dff`. [Windows workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43756654604). Native MSVC EXE SHA256 `8ff45ef8bdd714e2555978834ee757dd52f258ef26fd4d7bd4d61f9053f5eb53`.

| Check | Verified scope |
|---|---|
| Native VS2022 x64 build | Passed; packaged binary is this MSVC artifact |
| Module suites | 20/20 Windows, 16/16 portable and 16/16 ASAN+UBSAN passed on tested paths |
| Confidence continuity | Strong-only acquisition; fresh overlapping 70%-threshold retention; unrelated weak rejection; short-gap recovery; long-gap/geometry/disabled reset and nonfinite rejection passed deterministic fixtures |
| Display stability | Deterministic display-only grace, hard expiry, source/off reset and unchanged source freshness. Actual native GDI window painted dim/dotted LAST boxes on a brief empty result, cleared them with zero grace and returned to normal on a fresh result. It did not mutate actual detections |
| Tracking identity | Brief empty gap held only identity with no target/engaged/movement output. Compatible non-overlap shift returned the same ID; a long gap acquired a new ID. Stale/live/focus/activation/FOV and axis guards passed |
| Controller response | Given the same explicit measured 16 ms tick and known synthetic offset, Fast preset produced a larger initial step than default smoothing while respecting the 12-count event cap. This is a deterministic controller comparison, not a model/GPU FPS or actual Apex acquisition-time benchmark |
| Source/service isolation | Newest-pending initialization behavior preserved; confidence confirmation history could not leak across a source-generation reset. Actual ONNX integration contract models still yielded expected real decoded boxes |
| Settings | New stability/grace and identity settings round-tripped; legacy settings used compatible defaults; saved activation remains disarmed. Existing paths/profiles and bindings preserved |
| Own-window input probe | PASS actual Windows SendInput through default Hold, explicit Toggle and held Left Shift binding guards in OWN test window; inactive/disarmed toggle and unheld Shift rejected; observed toggle cursor delta=1,1; button/Shift released and cursor restored; no live game, Apex acceptance, trained-model GPU or aim accuracy tested |
| Existing regressions | Recorded pixels/actual DX11 preview/OCR; both synthetic-contract detection profiles; official OBS camera placeholder; minimized overlay; 120-second WARP stability; 60-second CPU ONNX; model recovery/cancellation and dependency negatives passed |
| Unchanged updater/launcher | Prior [preservation](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43745300804) / [launcher](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43745297183) tests remain evidence for unchanged scripts and their then-current payload, including stub child-stage orchestration, not a new user-PC repair |
| UI inspection | Actual CPU-rendered default/disconnected Tracking page reviewed. Not a Windows/game/GPU screenshot |

## Behavior changes

Detector decode permits weak internal candidates only when stabilization is enabled. The service's continuity filter requires the full user score for new acquisition; an already confirmed overlapping candidate can retain a real score down to 70% of that threshold. Confirmation expires after a 250 ms observation gap or source/model/geometry change. Actual source coordinates/scores remain unchanged; empty frames remain empty. Weak internal candidates cannot directly acquire new targets. An error resets continuity history.

Preview/desktop display has an independent bounded grace cache. A dim dotted LAST box may remain for the configured 120 ms default grace, bounded by maximum live display age plus grace. It never modifies the service snapshot, timestamps, controller inputs or activation. Errors/stops/changed sources and geometry clear history. Tracking has its own independent maximum frame age; visual grace does not extend it. Exact-frame inspection still uses actual source-aligned boxes.

The controller keeps only association identity across short gaps (default 250 ms), and recognizes a bounded center/size-compatible shift even without enough IoU. Missing/stale measurements produce zero movement and no current target. A fresh eligible associated box can resume the same ID; expired/incompatible association gets a new ID. This is local heuristic association, not game entity identity, target prediction or tracking through walls.

Fast lock + stable boxes explicitly sets response 8, zero smoothing, speed limit 800 counts/second, deadzone 1, identity retention 250 ms, detector stabilization, display grace 120 ms, cadence 16 ms, CUDA preference and live-display age 200 ms. It disarms, keeps binding/mode/radius/anchor and requires explicit activation. Saved previous axis settings are loaded unchanged until this action. First acquisition can use the current measured UI interval instead of an unconditional zero movement tick. The 12-count per-axis/event cap, focus/HWND/PID/F9/activation/live-profile/freshness gates and normal Windows SendInput remain.

A CPU/slow model or OBS buffering can still produce stale/no predictions. These changes cannot create a genuine current character detection from absent model output. They do not retrain or validate private model accuracy. No RTX/GPU FPS, actual Apex lock latency, sensitivity calibration or camera acceptance is claimed. These need the user's actual capture/model/hardware. Private weights are not distributed. Historical CPU model contract checks remain historical, not current gameplay accuracy proof.

[Setup](TRACKING.md) · [Detection/capture](GAME-ESP-README.md) · [CPU UI](previews/tracking.png). Raw records: [validation/stable-fast-tracking](validation/stable-fast-tracking).
