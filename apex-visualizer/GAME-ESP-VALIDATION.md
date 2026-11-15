# Tracking activation controls validation

Native source `328e5a9a51d206a3a1f3e261ff1607e9c4e074b5`. [Current Windows workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43753604252). Packaged MSVC EXE SHA256 `86575737e3d00cb00741ae70280da4c2ed6f6d4874d3fbfbce1ec178e9147671`.

| Check | Verified scope |
|---|---|
| VS2022 x64 native build | Passed; native MSVC binary packaged |
| Native / portable / ASAN+UBSAN modules | 19/19 Windows, 15/15 portable and 15/15 sanitizer suites passed |
| Activation controller | Hold/release, toggle press edges/no held-key repeats, restart, focus cancellation/no held-key refocus restart, F9, explicit START handoff, 10-second expiry and source/disarm clearing passed deterministic tests |
| Target controller | Nearest in-circle acquisition, sticky overlapping candidate despite a nearer unrelated box, empty/lost detection stop, stale/FOV/live/focus/arm gates, independent axes, zero-response reporting, smoothing and bounded movement passed |
| Preferences | Mode/key round-trip, invalid binding rejection, legacy Hold+Right mouse default and disarmed reload passed; runtime activation/window selection never saved |
| Actual Windows input | PASS actual Windows SendInput through default Hold, explicit Toggle and held Left Shift binding guards in OWN test window; inactive/disarmed toggle and unheld Shift rejected; observed toggle cursor delta=1,1; button/Shift released and cursor restored; no live game, Apex acceptance, trained-model GPU or aim accuracy tested |
| Window picker / desktop graphics | Actual isolated child-window minimized/class fallback selection and HWND/PID guards; click-through/non-activation, badge/circle/status painting, on/off, stale-source clearing and GDI resource checks passed. Not actual Apex compositor testing |
| Existing integration regressions | Real recorded pixels through actual DX11 preview/OCR; both synthetic-contract ONNX CPU detection profiles; official OBS camera placeholder; minimized overlay capture/inference; 120-second WARP stability; 60-second CPU compute; model recovery/cancellation/dependency negatives passed |
| GPU scope | CI runner lacks NVIDIA hardware; trained-model RTX execution/latency and gameplay performance are not verified by these results |
| Updater / launcher | Unchanged scripts retain prior [updater](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43745300804) and [launcher](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43745297183) preservation/handoff tests. Prior launcher orchestration used isolated child-stage stubs and its then-current payload, not a user-PC repair |
| UI review | Actual CPU-rendered disconnected/default Tracking UI reviewed; not a screenshot of the user's Windows/game |

## Changes

The previous arm checkbox did not activate input by itself; it still required right mouse. Tracking now has explicit Activation key/button and Hold/Toggle selection, Enable tracking, START LOCK and STOP / DISARM controls. START LOCK is a deliberate enable/Toggle action with a 10-second pending handoff to the selected game. It activates only when that game receives focus. Lost focus clears an active toggle; returning does not resume it automatically. F9 and invalid source/window/profile conditions cancel activation. Changing the key/mode disarms. Legacy defaults stay Hold+Right mouse. Saved preferences never restore arming, pending/active state or a selected HWND.

Every new activation resets selection so the nearest fresh eligible candidate is acquired. Overlapping detections retain it while eligible; target loss produces no invented position. The circle's box anchor and the model's actual boxes determine eligibility, not arbitrary objects that happen to be inside the circle. Centered/deadzone and disabled-axis states have explicit messages. Responsive follow is an optional explicit preset, not a silently changed calibration. Per-axis speed/smoothing/deadzone and the 12-count event cap remain.

Main controller readiness/focus/freshness/activation checks remain, and the native output path rechecks selected window/PID/focus, F9, explicit runtime activation and the selected held binding (Hold mode) before ordinary SendInput. Toggle operation deliberately does not require right mouse to stay down. No anti-cheat/input-filter bypass, automatic firing, game-memory access or motion extrapolation is added. Accepted Windows events do not prove actual Apex camera response.

The desktop FOV overlay now includes a second activation/lock/blocking status line. Capture includes an OBS-desktop troubleshooting panel. Documentation distinguishes a game minimizing on Alt-Tab from a genuinely wrong focused-game source, and describes correct display/Windows Graphics Capture/scene ordering/Program output/Virtual Camera checks. The app cannot remotely fix the user's OBS source; actual focused-game capture must be verified on that PC.

Existing detector profiles, low-latency preset/source-age expiry, recorded OCR, previews, DX11, saved settings and GPU dependencies remain. No accuracy/FPS/performance gain is claimed. Private weights are not distributed. Actual OBS Apex pixels, model target correctness, private-model CUDA performance, display alignment, selected key behavior inside Apex and actual camera acceptance/calibration remain user-PC tests.

[Tracking guide](TRACKING.md) · [Capture/detection guide](GAME-ESP-README.md) · [CPU UI](previews/tracking.png). Raw records: [validation/tracking-activation](validation/tracking-activation). Historical private-model CPU checks remain labeled as historical and are not gameplay accuracy proof.
