# Tracking: independent X/Y and FOV

The new **Tracking** category uses visible candidates from the existing Apex ONNX detector. It can emit bounded relative mouse movement through normal Windows `SendInput`. It does not automatically fire. The existing model pages, Roblox test profile, live preview, DX11 renderer, desktop boxes, old OCR and GPU setup remain.

## First test

1. Install the current tracking update into your existing app. Keep the GPU runtime; use the combined GPU/update launcher only if that copy still lacks CUDA dependencies.
2. Use Apex **Firing Range** in borderless/windowed mode. In OBS capture the same full game view, start Virtual Camera, then start the app's OBS receiver on Capture.
3. On Detection choose **Apex (primary, 416px)**, browse to your local Apex model and enable **Enable detection**. Verify actual model candidates and backend. The Roblox profile remains available for detection tests; this mouse controller requires the Apex profile.
4. Open **Tracking > Refresh Apex windows**, then select the actual Apex game window. The app enumerates normal visible window titles/IDs; it does not read game memory. The selected window is not saved across restarts.
5. Use the **X / horizontal**, **Y / vertical** and **FOV overlay** tabs under Tune controls. Configure X and Y independently. Each has an enable toggle, response strength, smoothing and speed limit. Start with the conservative defaults; game sensitivity changes the effect of mouse counts, so no universal calibration is claimed.
6. Open **Screen overlay**, click **Show on screen**, enable **Draw FOV circle**, and choose the correct monitor and radius. The circle appears immediately, centered on the monitor until a valid game window is selected; then it follows that game client. The radius is in client pixels, not camera FOV degrees. It can show without desktop bounding boxes enabled.
7. Put a correctly detected candidate slightly off-center inside the circle. Explicitly check **Arm mouse following**, switch focus to the selected Apex window, then hold the **right mouse button**. Release it to stop. **F9** globally disarms. If Windows cannot register F9, arming is disabled. **F8** hides/shows all overlay graphics; hiding graphics does not disarm mouse output.
8. Read Tracking status and **Windows input events accepted**. That counter means Windows accepted the events; it does not prove Apex used them. If tracking does not move the camera, use **Copy tracking diagnostics**. No blocked-input bypass is provided.

## X/Y response and timing

Response is proportional to image error, in mouse counts per client pixel per second. Speed is capped independently per axis, with an additional hard cap of 12 counts per axis per controller step. Smoothing is an exponential time constant on velocity. Disabling an axis clears its residual movement immediately. Center deadzone avoids small corrective steps. The anchor is a geometric point inside a box (default center), not a detected head/joint.

Candidates must have valid normalized boxes and lie inside the FOV at their chosen anchor. The nearest candidate is acquired, with overlap-based continuity on subsequent measurements. Track IDs are local associations, not game entity IDs. No target motion, wall-hidden player or friendly/enemy identity is inferred beyond the model's supplied pixels/boxes.

Mouse output also requires live camera capture, enabled Apex detection, selected-window focus, right-button hold, an available emergency hotkey and a fresh received frame. By default frames older than **250 ms** are rejected. The timestamp starts when the receiver creates its frame, before local preprocessing/inference; unknown OBS sender/display delay is not measured by this limit. CPU inference may be too slow and pause output. Lowering the existing model's detection interval can increase measurement cadence if hardware keeps up; no FPS gain is asserted.

Background capture/controller work continues while tracking is armed or its FOV is active; the main window's rendering can pause when minimized. Target focus and hold are checked again immediately before each input event. Native game input filtering, camera response, alignment, latency and effective sensitivity require testing on the actual PC. Event acceptance is not an Apex compatibility test.

## Persistence and stops

**Save tracking setup** writes `tracking-settings.json` beside the app's existing local settings. Arming and target window are never serialized. Successful loading starts disarmed. Source/model/profile changes disarm and reset the controller. No valid/fresh candidate, lost focus, released hold, unavailable F9 or an ordinary Windows input failure stops output. An input API failure disarms and appears in diagnostics. This is not a guarantee against arbitrary OS/driver failures.

Actual Windows tests, source/build hashes and remaining PC checks are documented in GAME-ESP-VALIDATION.md. No Apex-match automation, trained-model RTX performance, aim accuracy or performance improvement is claimed tested on the user's PC.
