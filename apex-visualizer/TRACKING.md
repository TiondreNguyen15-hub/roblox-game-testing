# Tracking activation and target lock

Tracking follows only fresh model detections inside the configured circle. The selected target is a local box association, not a game entity ID. It does not detect hidden players, invent missing targets or fire automatically. Mouse movement uses ordinary Windows SendInput. Windows accepting events does not prove Apex uses them or establish aim accuracy.

## Before starting

1. OBS must show the actual Apex game, not the desktop. Use borderless/windowed mode and verify the source while the game stays focused. See GAME-ESP-README.md for the desktop-capture troubleshooting steps.
2. Start OBS Virtual Camera, connect Capture, load the Apex-profile model in Detection and enable detection. Verify a real candidate box on a visible character. Low-latency live preset remains available.
3. Open Tracking and select the game window. The list refreshes every second and retains minimized games; restore the selected game before activation.
4. Open Screen overlay, Show on screen, and choose the game monitor. Enable the FOV circle. Without a valid selected game it defaults to monitor center; with a valid game it follows its client center. Detection boxes require the full capture to match that monitor; OBS crops/borders/scene scaling can misalign the mapping.

## START LOCK respects your selected activation mode

Choose the activation key/button and **Hold selected key** or **Toggle** first. **START LOCK never changes that selection.**

- **Hold:** START LOCK enables tracking, then switch to the selected game and hold the chosen physical key/button. Focusing the game alone does not activate movement. Releasing the key stops movement immediately; there is no Toggle latch or 10-second pending activation in Hold mode.
- **Toggle:** START LOCK keeps Toggle mode and requests a 10-second handoff to the selected game. Switch to that game within 10 seconds to activate; the configured key toggles the session off. Lost focus cancels the active toggle, and returning does not resume it automatically.

STOP / DISARM or F9 stops and disarms either mode. Choosing a new key/mode disarms as before. The regular Enable tracking checkbox and in-game activation-key path remain. START does not make an undetected object trackable or force Apex to accept input: actual fresh eligible model boxes are required. No selected window or active/pending/armed state is restored from saved preferences.

## Activation key / button

Choose **Right mouse**, **Mouse 4**, **Mouse 5**, **Left Alt**, **Left Shift** or **Left Ctrl**, then select a mode:

- **Hold selected key:** enable tracking, focus the selected game and hold the chosen key. Releasing it stops movement.
- **Toggle: press to start / stop:** enable tracking, focus the selected game and press the chosen key. Release before the next press; holding a key does not repeatedly toggle. Focus loss cancels a toggle. A key already held when arming/refocusing does not trigger a new toggle.

Changing the binding or mode disarms tracking. Saved settings include the binding/mode, but never an armed, pending, active or selected-window state. Legacy files without the new fields load as Hold + Right mouse, disarmed.

## Stable boxes and faster acquisition

Click **Fast lock + stable boxes** before START LOCK. This explicit preset enables X/Y, uses response 8, zero smoothing, speed limit 800 counts/second, deadzone 1 pixel, and 250 ms identity retention. It also enables confirmed-detection stabilization, 120 ms visual grace, 16 ms detection scheduling, CUDA preference and the 200 ms live-display age limit. It preserves the selected activation binding/mode, FOV radius and anchor, and disarms until START or an explicit enable. Save tracking setup and Detection > Detection tuning and saving > Save enemy setup to persist both groups.

The controller can use the measured current frame interval on the first active update; it no longer adds a mandatory zero-movement acquisition tick. The 12-count per-axis/event cap remains. This is a controller response change, not proof of faster model/GPU inference or instant Apex lock. Game sensitivity still determines the camera response.

Confidence stabilization uses the full configured score to acquire a new detection. A previously confirmed overlapping box can retain an actual fresh score down to 70% of that acquisition score for up to a 250 ms observation gap. It uses actual measured coordinates/scores; it does not fabricate a missing model output. Disable Stabilize confirmed detections in Detection tuning if it retains unwanted boxes.

Across a brief empty/stale measurement gap, tracking retains only the local association identity for the configured Brief-gap identity retention time (default 250 ms). It produces zero movement during missing/stale results. A compatible new fresh measured box can reuse that identity, including a small bounded center shift with compatible size. A longer gap, incompatible geometry or a newly acquired unrelated candidate gets a new identity. Model association is heuristic, not a real game entity ID. The maximum received-frame age remains independent and is not extended by identity retention.

A dim dotted **LAST** box can remain on the display for the Visual gap grace time (default 120 ms), bounded by maximum live display age plus that grace. It is explicitly historical display data. It never feeds the tracking controller, refreshes a source timestamp, changes acquisition confidence or keeps activation/movement engaged. Tracking uses actual service detections and its own independent freshness limit. Failed/stopped/changed sources clear the display history. Exact analyzed-frame mode still shows boxes on their actual source image.

## Limits and diagnostics

Responsive follow preset remains as the milder option. Existing X/Y/FOV, anchor, deadzone and timing controls remain. Fast preset is not applied silently to existing saved axis settings. Disabled/zero-response axes are reported; a centered target needs no movement.

Focus, HWND/PID, activation, F9, live nonrecording Apex profile and receiver freshness gates remain. Hold mode rechecks the selected physical binding before ordinary SendInput; Toggle requires runtime activation. No input-filter/anti-cheat bypass, automatic firing or game-memory access is added. Accepted Windows events do not prove actual Apex camera response.

Tracking shows activation, target/gap and blocking status. The FOV overlay includes the same status when displayed. Copy tracking diagnostics includes the activation binding/mode, model backend and received-frame age; Copy detection diagnostics also includes acquisition score, stabilization, fresh lower-score retention count and visual grace. F8 only changes graphics; F9 disarms.

Slow/failed inference still produces stale/no-target status. A display grace cannot make a slow CPU model fast, and an actual missing prediction cannot be recovered as a fresh target without a new model output. Unknown game/OBS sender buffering remains outside receiver age. Actual capture, model accuracy, RTX performance, sensitivity and Apex input acceptance require user-PC testing.
