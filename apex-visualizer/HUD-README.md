# Recorded HUD text reader

This build adds actual text recognition for local video recordings. It uses **Windows OCR on CPU**, independently of the existing CUDA/ONNX hardware benchmark. The bundled compute_validation.onnx is still a mathematical test; no Apex-trained detector or gameplay model is claimed.

## Install without moving GPU files

Leave your current working demo open. Extract **all** files from `Apex-HUD-Update.zip`, then double-click **Update-HUD.cmd**. The updater finds the running demo, closes it normally, verifies the new executable's SHA256, backs up the old executable and replaces it in the same app folder. It keeps `runtime/gpu`, existing models and saved settings. If no unique running app is found, it checks the last known Compute-Fix folder or asks you to select apex_visualizer.exe. The old executable is retained under `update-backups`.

The app reopens. No GPU-library download or CUDA reinstall is needed.

## First test

1. Open **HUD Reader** in the left sidebar.
2. Click **Open recording...**. Choose `HUD-Sample.mp4` in your app folder, or your own local H264 MP4 recording. The included sample is original generated test text, not gameplay.
3. Click **Pause clip**. A finished recording also keeps its last frame available.
4. Drag a rectangle over only the text you want to read. For the sample, the full picture is acceptable; set **Text upscale** to **1**.
5. Select **Ammo counter**, **Match-result banner**, or **Raw text**, then **Read current frame**.
6. For continuous sampling, enable **Read HUD while clip plays** and resume the clip. The default interval is 750 ms; this is not an every-frame detector.
7. **Save HUD setup** preserves your normalized region, mode, interval and scale in a separate `hud-settings.json`. Existing app configuration is unchanged. **Export OCR CSV...** lets you select a file and exports the latest 600 observations for the current source.

For actual gameplay, start with a tight region around the magazine digits. Exclude weapon names, icons, distance/health values and other text. A new recording resets the observation history; export before changing sources if you want to keep it. The region is normalized to the image, but different HUD layouts, resolutions, languages and UI scaling still need calibration.

## Readings and accuracy

- Raw text is returned by Windows OCR from actual captured pixels. Nothing is filled in from simulated player data or assumed gameplay state.
- Ammo mode accepts only an isolated 1–3 digit counter, or an explicit `magazine / reserve` pair with up to four reserve digits. Ambiguous text is rejected. Letters such as O or B are not guessed into numbers.
- Result mode recognizes exact English `YOU ARE THE CHAMPION`, `CHAMPION` or `SQUAD ELIMINATED` text. These are unverified banner candidates; no win/loss or competitive statistics are updated.
- Parsed values are labeled **candidates**, not verified facts. Windows OCR does not expose a confidence score in this API, so none is invented. A wrong region can read another number correctly but give it the wrong meaning. Compare results with the paused frame.
- Raw text mode can export other visible HUD text without making semantic claims.
- Each CSV row includes the source, frame sequence, original source timestamp, measured crop/recognition latency, mode, region, raw text and any parsed candidates. Formula-like text is escaped for spreadsheet safety. OCR latency excludes backend initialization and is not CUDA model latency or display FPS.

Only local recordings are analyzed in this phase. Existing OBS capture remains available on the Capture page. No game injection, protected-memory access, input automation or anti-cheat interaction is added.

## Backend and performance

Windows 10/11 native builds use C++/WinRT Windows.Media.Ocr. An installed OCR language is required; user-profile languages are tried, then en-US. If unavailable, the actual error is shown and **Copy OCR diagnostics** remains available. The non-MSVC cross-check binary explicitly reports that the projection is unavailable; downloadable executables are the actual Visual Studio native build.

Recognition runs on its own MTA worker with one replaceable pending frame. The UI receives a small snapshot, and history is capped at 600 readings. Changing the source or region invalidates stale in-flight results. Pixel processing crops first and bounds the recognition image to 1600x1600, with configurable nearest-neighbor upscale. Recorded playback can pause/resume for region selection; pausing is not applied to OBS camera timing.

The first implementation is not GPU-accelerated OCR, a trained Apex object detector, or proof of gameplay-reading accuracy. GPU inference remains verified separately by the existing benchmark. Your own footage is required to calibrate and evaluate actual HUD accuracy.

[Actual native tests and remaining checks](HUD-VALIDATION.md).
