# Game ESP: local enemy-model capture analysis

The sidebar's **HUD Reader** entry is now **Game ESP**. This page loads your local trained model and shows a live captured preview with the latest candidate boxes. Exact analyzed-frame viewing remains optional. Existing recording text tools remain in a collapsed section; JSON/simulated visuals, settings, OBS capture, DX11 rendering and compute benchmarks remain.

## Install and start

1. **For your confirmed CPU-only runtime:** leave the one demo open if possible (otherwise select its installed EXE when prompted), extract the complete update ZIP, and run **Update-And-Enable-GPU.cmd**. It captures that executable path, runs the existing pinned GPU repair helper, then updates that same copy. Up to 1.4 GB may download; cached packages are reused. The optional normal update below preserves installed files without repairing a missing GPU runtime. Installation is not proof of model GPU execution.

   For a copy that already has working GPU dependencies, leave your current working demo open. Download **Apex-Game-ESP-Update.zip**, right-click it and choose **Extract All**. Open the extracted folder and double-click **Update-Game-ESP.cmd**. The updater locates your running demo, closes it normally, backs up its executable, updates it and reopens it. Keep your existing GPU libraries; no GPU reinstall is required.
2. Open **Game ESP**, then **Browse enemy ONNX model**. Select your local `SingularityMetaV3_recovered.onnx` file. If it is inside a ZIP, extract the ZIP first. The private trained weights are deliberately not included in the public download.
3. Choose **Open recording for enemy analysis**, or start your existing OBS receiver on **Capture** and return to **Game ESP**. Browse automatically enables incoming-frame analysis. **Detect enemies in current frame** also works with a paused recording.
4. Adjust **Detection score** and **Overlap suppression** if needed. Default score is 0.50. **Save enemy setup** preserves the local model path, CUDA preference, thresholds and analysis interval in `%LOCALAPPDATA%/ApexVisualization/enemy-settings.json`.
5. Read **Backend**. Only `CUDA EP execution profiled (CPU ops permitted)` means this detector actually executed a non-copy CUDA node. It is separate from the math-model validation. CPU fallback includes its reason. Use **Copy Game ESP diagnostics** if initialization fails.

The preview now updates from live capture independently of model completion. The previous default preview waited for each model result, so a 150 ms detection interval alone limited its visible refresh to about 6.7 updates/second even with fast inference. Keep **Show exact analyzed frame (updates at model speed)** unchecked for the smooth live view. Enable it to inspect boxes on their exact source pixels. Live boxes are delayed model results; result age and processing latency are displayed separately from actual capture FPS. This removes that preview coupling, but does not establish an RTX speedup or guarantee against GPU contention. The queue holds at most one pending frame and drops older pending work, rather than building an increasing backlog. Detection interval limits submissions; it is not an inference FPS claim. To unload the model fully, close the app. Browse again to reload a model replaced at the same path.

If boxes still update slowly, check **Backend**. `ONNX CPU` means this model is not running on the RTX 3070, even if a previous math-model test passed in another app folder. **Copy Game ESP diagnostics** now includes actual capture FPS, UI frame time, model processing time and result age so these can be distinguished. The updater preserves GPU files in the selected app folder. If diagnostics show that its GPU runtime files are missing, the update includes an optional **GPU-Reinstall/Reinstall-GPU.cmd** helper: leave the demo open and run that helper from the extracted update. It targets the currently running copy, installs pinned local dependencies (up to 1.4 GB; cached downloads reused), then reopens it. It does not run automatically, and installation alone does not verify CUDA execution. A GPU initialization/model error requires diagnosing that actual error, not assuming reinstall is the solution.

## If the combined launcher did not reopen the app

Use the current **Update-And-Enable-GPU.cmd** from a fully extracted update. It targets the selected app explicitly, installs GPU files without opening an intermediate copy, then updates and launches it once. It verifies that the actual app window stays running for 1.5 seconds before reporting success.

If your demo can open, leave only that copy open. If it is already closed, the launcher reuses a previously remembered target or asks you to choose the installed **apex_visualizer.exe**. Select your usual app, not the update's payload or a backup. No manual PowerShell command is required. After target selection it remembers that path for a retry.

If startup fails, the launcher reports the exit/failure and saves **launch-failure.txt** beside that executable, including recent own-app logs and any accessible matching Windows crash record. Send the error text or that file. The original reported reopen failure did not include an exit code, so its exact cause remains unknown; successful launch checks here do not prove this PC has been repaired or guarantee a later model/driver run.

## Roblox test profile (Apex stays primary)

1. Capture Roblox in OBS, fit the full game view to its canvas and start Virtual Camera. In the app use **Capture > Start OBS Virtual Camera**.
2. Open **Game ESP > Model profile > Roblox (test, YOLO 640px)**.
3. **Browse enemy ONNX model** and choose your local **RobloxAIO-harlan.onnx**. Trained weights are separate and are not included in the public download.
4. Enable **Analyze incoming capture frames**. Leave **Show exact analyzed frame** unchecked for live preview; optionally enable the desktop overlay for the selected monitor.
5. Check actual Backend and candidates against visible targets. Only a profiled CUDA execution status proves GPU work; CPU fallback remains explicit. Accuracy and Roblox-specific target coverage require footage, not a black-image sanity test.
6. **Save enemy setup** stores the Roblox model path and thresholds separately. Switching profiles also saves the previous setup; it clears old detections and disables analysis until you select a model or enable it again. Select **Apex (primary, 416px)** to return. The app starts on the Apex profile each launch.

Existing Apex settings remain in `enemy-settings.json`; Roblox uses `roblox-enemy-settings.json` under the same local application folder. Legacy Apex files without a profile field still load as Apex. Unknown/mismatched profile settings are rejected before mutating the selected configuration. Model/profile tensor mismatches are shown explicitly; failed models do not reload continuously until you retry or change options.

The inspected Roblox export uses FP32 `images` [1,3,640,640] -> `output0` [1,5,8400], Ultralytics YOLO single-class channel-major `(centerX,centerY,width,height,score)` pixel output. Its metadata labels class 0 **Enemy**. The adapter uses RGB/[0,1]/gray114 centered letterbox preprocessing, inverse coordinate mapping, score filtering and IoU NMS. It examines all 8400 rows for validity, keeps the top 1000 scored candidates before NMS and caps displayed detections at 300. This class name does not establish real enemy/friendly correctness in every Roblox experience.

## Desktop overlay on your main monitor

1. Put Apex in **Borderless Window** or windowed mode on your main monitor. Exclusive fullscreen is not supported by this desktop window.
2. In OBS, capture the full game view. Prefer **Window Capture** of the game, fit it to the OBS canvas, and start **Virtual Camera**. Cropped scenes, a webcam inset or extra borders can misalign detection boxes.
3. In the demo, **Capture > Start OBS Virtual Camera**, then **Game ESP > Browse enemy ONNX model** and choose your local model. Keep **Analyze incoming capture frames** enabled.
4. Check **Desktop overlay (live capture)**. Leave **Overlay monitor** set to **Primary monitor** for your main screen, or choose another display. The overlay starts off each time you launch the app.
5. Switch back to the game. The purple **Game ESP** badge and any actual **Enemy?** boxes appear on the selected monitor. You can minimize the demo; its capture and inference continue while the overlay is enabled.
6. Press **F8** to hide/show the overlay while the game has focus. If another app owns that hotkey, Game ESP reports it; use the checkbox instead. Close the demo to remove the overlay entirely.

The overlay is a separate click-through, non-activating Windows desktop window, so it does not intercept game clicks or keyboard focus. It displays captured-image detections with capture/inference delay; boxes do not predict target motion. It clears boxes one second after the last completed analysis and hides when live capture stops or when a recording is selected. A visible badge without boxes means no fresh accepted detections, not proof of correct enemy recognition.

**Fit capture aspect ratio to monitor** centers the captured view with letterboxing/pillarboxing. Turn it off only if your displayed game actually stretches the capture to fill the monitor. The overlay assumes the full capture corresponds to that monitor; it does not automatically track a smaller game window, undo OBS crops or calibrate scene transformations. Use borderless fullscreen on the same selected display for the first test.

The app requests Windows capture exclusion for its overlay (Windows 10 2004+/11). If Windows rejects it, the status warns you to use OBS Window Capture. Actual OBS scene/compositor compatibility still needs checking on your PC; use Window Capture if Display Capture includes the overlay. The DX11 preview remains available for comparison.

## Model contract and limits

The supplied file was inspected and checked: FP32 input `in_00000` `[1,3,416,416]`; FP32 output `out_00000` `[1,300,6]`; end-to-end rows `(x1,y1,x2,y2,score,class)`. The adapter uses bilinear RGB NCHW scaling to `[0,1]`, centered aspect-preserving 416px letterboxing with gray 114 padding, restores boxes to original-frame coordinates, clips them and applies score filtering/IoU suppression. This preprocessing follows the observed YOLO-style contract; full training/preprocessing documentation was not supplied. That is the **Apex** profile. The separate **Roblox** contract is documented above; other shapes are rejected with an error.

The model's actual metadata label is `{0: 'item'}`. You described it as an enemy detector; the UI therefore labels results **Enemy?**. Enemy/friendly classification and accuracy have not been independently established. Scores are model outputs, not calibrated accuracy estimates. No weapons/ammo detector was added. No health, distance, skeleton or hidden-player information is invented from the detector output.

This is analysis of externally captured pixels, shown in the app preview or its separate desktop overlay. It does not inject into Apex, read its memory, bypass anti-cheat or automate gameplay. Only visible image content can be analyzed. Existing Windows OCR text tools remain CPU-based.

GPU inference uses the existing pinned local ONNX/CUDA/cuDNN loader, renderer adapter identity, TF32 disabled, one CPU thread and a 1 GiB CUDA allocator limit (not a limit on all process GPU memory). CPU shape/operator work is permitted and honestly reported. Provider initialization or ordinary inference exceptions fall back to CPU; native driver/DLL process crashes cannot be made recoverable with a C++ exception handler.

See **GAME-ESP-VALIDATION.md** for actual tests and limits. The generated test fixture in source/tests is explicitly synthetic and is never presented as the trained detector or bundled into runtime models.
