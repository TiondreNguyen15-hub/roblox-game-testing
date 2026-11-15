# START LOCK Hold-mode regression fix

Native source `aa01b7fdaf8d71b14d5ce6aee4ec17d330272b1a`. [Complete Windows workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43759430688). Packaged native MSVC EXE SHA256 `205a2586a97cdd07840291132e802e8c99b7df1b0b6a8ce91d6918062e389e25`.

| Check | Verified scope |
|---|---|
| Native Windows VS2022 x64 | Passed; native artifact is packaged |
| Native / portable / ASAN+UBSAN module suites | 20/20 Windows, 16/16 portable and 16/16 sanitizer suites passed |
| START action mode/key preservation | Shared menu action used by START LOCK was tested in Hold and Toggle modes: selected enum/key unchanged, explicitly armed and correct runtime action emitted once |
| Hold START controller regression | Hold START created no pending toggle/latch. Focus alone did not activate. Selected key-down activated; key-up stopped; missing focus and F9 prevented activation. Mode remained Hold |
| Toggle / previous activation tests | Press edges, no held-key repeat, focus cancellation/refocus guard, existing 10-second START handoff/timeout, source/disarm and saved preference behavior passed |
| Existing detection/stability/controller suites | Confirmed-only score hysteresis, weak-new rejection, display-only LAST grace, identity retention/no missing-target movement, bounded faster first step and per-axis/freshness/FOV checks passed |
| Native input probe | PASS actual Windows SendInput through default Hold, explicit Toggle and held Left Shift binding guards in OWN test window; inactive/disarmed toggle and unheld Shift rejected; observed toggle cursor delta=1,1; button/Shift released and cursor restored; no live game, Apex acceptance, trained-model GPU or aim accuracy tested |
| Existing Windows integrations | Actual recording/OCR/DX11 preview, both synthetic-contract ONNX CPU detector profiles, official OBS placeholder/background overlay, WARP stability, CPU compute/model recovery/cancellation/dependency negatives passed |
| Updater/launcher evidence | Unchanged scripts retain prior [updater preservation](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43745300804) and [launcher](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43745297183) checks of their then-current payload/stubbed stage orchestration; not a new user-PC repair |
| UI review | Actual CPU-rendered disconnected/default Tracking page reviewed; not a user-PC screenshot |

## Fixed behavior

START LOCK previously overwrote `activationMode` with Toggle. The button now calls a shared menu action which only arms and emits START, preserving the mode and binding. The runtime request supports Hold explicitly without creating a pending Toggle activation. Hold mode requires the actual configured key/button while the selected game is focused; release stops movement. Toggle retains the existing explicit START handoff and its 10-second deadline. Mode-aware UI guidance reflects those differences.

Existing saved preferences are loaded as before. The application cannot infer whether a previously saved Toggle selection was intentional; select Hold again after updating if an older START action changed it, then Save tracking setup. Active/pending/armed state and selected HWND never persist.

Detection model/threshold behavior, stability, fast preset, GPU/runtime dependencies, receiver/renderer and normal Windows output implementation are unchanged by this fix. F9, focus, selected HWND/PID, source/freshness/FOV and bounded movement gates remain. Actual Apex input acceptance, current user OBS scene, private model recognition/GPU speed and hardware performance still require PC evidence. No accuracy/performance improvement is claimed here and private model weights are not redistributed.

[Tracking instructions](TRACKING.md) · [CPU UI](previews/tracking.png). Raw records: [validation/hold-start-fix](validation/hold-start-fix).
