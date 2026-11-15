# Tracking-window picker and live detection latency validation

Native source `319a3ed5adf8c563c40e1aeb965d7ebf9c2c35d3`. [Current complete Windows workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43751674773). Packaged native MSVC EXE SHA256 `3087748741b37c3f15465100900a199d04d36242f5f762a3b61944df207e1625`.

| Check | Verified scope |
|---|---|
| Native Windows VS2022 x64 build | Passed; this native executable is packaged |
| Native module suites | 19/19 passed, including picker, scheduler, frame age and existing regressions |
| Portable and ASAN/UBSAN suites | 15/15 each passed on tested paths |
| Window picker | Actual isolated child process created minimized Apex-titled and renamed Respawn001-class windows. Both enumerated; minimized selection explained restore requirement; restored class fallback validated. Unrelated/own windows and PID mismatches rejected. This is not enumeration of an actual Apex installation |
| Latest-frame scheduling | Deterministically blocked detector-initialization fixture queued frames 1,2,3; only newest frame 3 was inferred after release. Single pending replacement and source reset passed. The fixture is explicitly synthetic, not an ONNX/GPU/FPS test |
| Box age | A 500 ms-old source frame was stale despite fresh inference completion; fresh/expired/future/missing timestamps and unavailable results tested. Actual Win32 overlay cleared an old-source box even when completion was recent |
| Settings | Per-profile 16 ms cadence and 200 ms age setting round-tripped; legacy version-1 files load with 250 ms default. Existing paths/profiles are preserved |
| Desktop overlay | Separate actual window, immediate badge/circle without capture/model/focus, on/off, click-through/non-activation, independent boxes, stale-source clearing and 200-repaint GDI cleanup passed |
| Capture/detection integration | Real recorded pixels -> preprocessing -> synthetic-contract ONNX CPU model -> DX11 preview passed for both Apex and Roblox formats. Wrong profile rejected. Official OBS placeholder and minimized-overlay updates passed. No private-model accuracy or actual-game claim |
| Other existing regressions | Windows OCR preview, 120-second WARP stability, 60-second CPU ONNX validation, model benchmark/recovery, cancellation and dependency-error paths passed. GPU tests remain unavailable on a runner without NVIDIA hardware |
| Own-window input probe | PASS actual Windows SendInput through focus/hold guard in OWN test window; observed cursor delta=1,0; button released and cursor restored; no live game, Apex acceptance, trained-model GPU or aim accuracy tested |
| Updater/launcher | Unchanged scripts retain [prior updater regression](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43745300804) and [prior launcher regression](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43745297183). These previously checked replacement/backup, model/GPU/settings preservation, then-current native payload startup and stubbed stage handoff; not a new user-PC repair test |
| UI review | Actual CPU-rendered Tracking and Detection ImGui pages reviewed. They show disconnected/default state, not Windows/game or GPU proof |

## Resulting behavior

Tracking refreshes the recognized window list once per second while its page is open. Minimized games remain selectable, and the selected window's state explains that it must be restored. The picker matches an Apex title or native Respawn001 class, and Refresh windows lives on its own row so it is not squeezed off compact layouts. Empty lists explain the condition instead of showing a blank popup. Window selection stays explicit and never arms output. All focus, hold, F9, PID, freshness and bounded movement gates remain.

Detection adds **Low-latency live preset**: 16 ms submission scheduling, CUDA preference, live preview and 200 ms maximum live box source age. Existing confidence, overlap, model and tracking settings remain. Save enemy setup persists it. Existing saved cadence is loaded as before until this explicit action changes it; normal legacy maximum age defaults to 250 ms. The model worker replaces its initialization-triggering frame with the newest pending frame before running inference. There is no growing pending queue.

Live preview and desktop boxes now expire from their source frame's receiver timestamp, including local queue and inference time. Previously completion time could make a slow result appear fresh. Stale boxes clear rather than trailing targets. Exact analyzed-frame inspection continues to show matching boxes on their actual source image. A strict age limit may hide boxes from a slow CPU model; this does not make that model faster. Sender/game/OBS buffering before receiver arrival remains unknown.

No motion extrapolation, fabricated targets, model-weight changes or game-memory access is added. CUDA-requested and CUDA-verified remain distinct. No GPU latency/FPS/accuracy improvement is claimed without PC measurements. Current private model, actual Apex enumeration/focus/compositor behavior, OBS buffering and RTX 3070 execution/latency must be checked on the PC. Model accuracy needs representative annotated footage; historical private CPU contract checks are not that evidence.

[Setup guide](GAME-ESP-README.md) · [Tracking](TRACKING.md) · [Tracking CPU UI](previews/tracking.png) · [Detection CPU UI](previews/detection.png). Raw records: [validation/tracking-detection](validation/tracking-detection). Trained weights are not distributed.
