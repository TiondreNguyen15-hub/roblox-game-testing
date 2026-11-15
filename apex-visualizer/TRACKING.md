# Tracking activation and target lock

Tracking follows only fresh model detections inside the configured circle. The selected target is a local box association, not a game entity ID. It does not detect hidden players, invent missing targets or fire automatically. Mouse movement uses ordinary Windows SendInput. Windows accepting events does not prove Apex uses them or establish aim accuracy.

## Before starting

1. OBS must show the actual Apex game, not the desktop. Use borderless/windowed mode and verify the source while the game stays focused. See GAME-ESP-README.md for the desktop-capture troubleshooting steps.
2. Start OBS Virtual Camera, connect Capture, load the Apex-profile model in Detection and enable detection. Verify a real candidate box on a visible character. Low-latency live preset remains available.
3. Open Tracking and select the game window. The list refreshes every second and retains minimized games; restore the selected game before activation.
4. Open Screen overlay, Show on screen, and choose the game monitor. Enable the FOV circle. Without a valid selected game it defaults to monitor center; with a valid game it follows its client center. Detection boxes require the full capture to match that monitor; OBS crops/borders/scene scaling can misalign the mapping.

## Start with the screen button

Click **START LOCK**, then switch to the selected Apex game within **10 seconds**. This explicit button enables tracking and selects Toggle mode. It waits for game focus before activating. The nearest fresh detected candidate whose configured box anchor is inside the circle is acquired; overlapping detections retain that candidate while it remains eligible. On a new activation, acquisition starts again from the nearest candidate.

Click **STOP / DISARM** or press **F9** to stop and disarm. The configured activation key also toggles an active Toggle session off while the selected game is focused. Losing focus cancels the toggle; returning to the game does not automatically restart it. A missed 10-second handoff expires the request. Invalid window/source/profile/model state also clears activation.

The button does not force Apex to accept mouse input and does not make an undetected object trackable. If the capture contains desktop pixels or the model has no accepted character box in the circle, there is no target to follow. Target loss or stale results stop movement; a fresh eligible detection can be acquired again while activation remains valid.

## Activation key / button

Choose **Right mouse**, **Mouse 4**, **Mouse 5**, **Left Alt**, **Left Shift** or **Left Ctrl**, then select a mode:

- **Hold selected key:** enable tracking, focus the selected game and hold the chosen key. Releasing it stops movement.
- **Toggle: press to start / stop:** enable tracking, focus the selected game and press the chosen key. Release before the next press; holding a key does not repeatedly toggle. Focus loss cancels a toggle. A key already held when arming/refocusing does not trigger a new toggle.

Changing the binding or mode disarms tracking. Saved settings include the binding/mode, but never an armed, pending, active or selected-window state. Legacy files without the new fields load as Hold + Right mouse, disarmed.

## Lock response, limits and diagnostics

**Responsive follow preset** explicitly enables both axes and sets response 4, smoothing 15 ms, maximum speed 500 counts/second and deadzone 2 pixels. It does not overwrite the user's configuration until clicked/saved. Existing X/Y sliders, radius, anchor, maximum received-frame age and independent axis controls remain. No universal game-sensitivity calibration is claimed.

The geometric box anchor (default center) must be inside the FOV. It is not a detected joint. The controller retains overlapping boxes by IoU and reacquires when no eligible association remains. It does not extrapolate hidden or lost targets. Disabled/zero-response axes are reported; a centered target needs no movement.

Motion remains speed-limited, smoothed and capped at 12 counts per axis per controller step. Focus, selected HWND/PID, activation, F9, live nonrecording Apex profile and receiver freshness gates remain. The selected binding is sampled again immediately before a Hold-mode event; Toggle mode requires explicit runtime activation. Actual Apex camera response and sensitivity must be checked on the PC; no input-filter/anti-cheat bypass is added.

The activation and target statuses are visible on Tracking. When its FOV graphic is displayed, the desktop overlay also shows the activation/blocking/lock status. **Copy tracking diagnostics** includes binding, mode, activation state, model backend, frame age and accepted Windows events. F8 only changes overlay graphics; F9 disarms input.

Receiver frame age includes local processing but not unknown sender/OBS delay. Slow/failed inference produces stale/no-target status instead of moving on an old box. Existing previews, model profiles, old OCR, DX11 renderer, GPU setup and low-latency detection controls are preserved.
