# Workspace: capture, detection and tracking


The sidebar's **HUD Reader** entry is now **Detection**. This page loads your local trained model and shows a live captured preview with the latest candidate boxes. Exact analyzed-frame viewing remains optional. Existing recording text tools remain in a collapsed section; JSON/simulated visuals, settings, OBS capture, DX11 rendering and compute benchmarks remain.

## Navigation

The app opens on **Home**, with actual capture/model status and three setup steps. Use **Capture > Detection > Tracking** in order. **Appearance**, **System** and **Settings** are grouped under Manage; local JSON and simulated visuals remain under **Data lab** in Advanced.

**Detection** is the former Game ESP page. Live and exact-frame previews, Apex/Roblox profiles, model selection, overlay and recorded text reader remain. Open **Detection tuning and saving** for CUDA preference, score/overlap/cadence and Save enemy setup; open **Model details and diagnostics** for hashes and copied results. **Recorded text reader** preserves the OCR controls.

**Tracking** has visible setup/activation steps and separate **X / horizontal**, **Y / vertical** and **FOV overlay** tabs. Detailed counters and copied tracking status are under **Tracking status and diagnostics**. Activation rules, F9 and saved settings are unchanged.

**System** opens on Performance. Choose **Compute checks** for validation, custom-model benchmarks, cancellation and detailed results. Those tests remain distinct from your detection model. **Settings** saves the existing configuration; **Appearance** controls the accent and window size. Existing saved dimensions are preserved; **Expand layout** switches to the roomier workspace without silently overwriting your preferences.

Capture selection, model loading and tracking arming remain explicit actions. Home buttons only open the relevant page; they do not arm tracking. Backend badges describe actual detector results, not driver detection or presumed GPU success.

## Tracking category

**Tracking** now has separate **X / horizontal**, **Y / vertical** and **FOV overlay** sections. Configure the existing Apex detector/capture first; then Refresh Apex windows and select the actual game window. The FOV circle can show without desktop boxes. Arming is off at startup; mouse following requires the selected game focused and the right mouse button held. Release the button to stop; **F9** disarms globally. **F8** hides/shows overlay graphics without disarming. Settings load disarmed and do not restore a window selection.

X/Y enable, proportional response, smoothing and speed limits are independent. FOV is a client-pixel radius. By default results older than 250 ms from receiver creation are rejected; unknown OBS sender latency is not included. Only the Apex model profile can drive mouse output. Windows input acceptance is reported separately from whether Apex actually responds. Use **Copy tracking diagnostics** for failures; no input bypass or automatic firing is added. Start in Firing Range and verify the model is detecting the intended visible targets. The model's class label alone does not establish friend/foe accuracy.

Save/Load tracking setup is separate from model and renderer settings. Full instructions are in TRACKING.md (included in the source/full archive and update payload).

## Install and start

1. **For your confirmed CPU-only runtime:** leave the one demo open if possible (otherwise select its installed EXE when prompted), extract the complete update ZIP, and run **Update-And-Enable-GPU.cmd**. It captures that executable path, runs the existing pinned GPU repair helper, then updates that same copy. Up to 1.4 GB may download; cached packages are reused. The optional normal update below preserves installed files without repairing a missing GPU runtime. Installation is not proof of model GPU execution.

   For a copy that already has working GPU dependencies, leave your current working demo open. Download **Apex-Game-ESP-Update.zip**, right-click it and choose **Extract All**. Open the extracted folder and double-click **Update-Game-ESP.cmd**. The updater locates your running demo, closes it normally, backs up its executable, updates it and reopens it. Keep your existing GPU libraries; no GPU reinstall is required.
2. Open **Detection**, then **Choose ONNX model**. Select your local `SingularityMetaV3_recovered.onnx` file. If it is inside a ZIP, extract the ZIP first. The private trained weights are deliberately not included in the public download.
3. Choose **Open recording for enemy analysis**, or start your existing OBS receiver on **Capture** and return to **Detection**. Browse automatically enables incoming-frame analysis. **Analyze current frame** also works with a paused recording.
4. Adjust **Detection score** and **Overlap suppression** if needed. Default score is 0.50. **Save enemy setup** preserves the local model path, CUDA preference, thresholds and analysis interval in `%LOCALAPPDATA%/ApexVisualization/enemy-settings.json`.
5. Read **Backend**. Only `CUDA EP execution profiled (CPU ops permitted)` means this detector actually executed a non-copy CUDA node. It is separate from the math-model validation. CPU fallback includes its reason. Use **Copy Detection diagnostics** if initialization fails.

The preview now updates from live capture independently of model completion. The previous default preview waited for each model result, so a 150 ms detection interval alone limited its visible refresh to about 6.7 updates/second even with fast inference. Keep **Exact analyzed frame (updates at model speed)** unchecked for the smooth live view. Enable it to inspect boxes on their exact source pixels. Live boxes are delayed model results; result age and processing latency are displayed separately from actual capture FPS. This removes that preview coupling, but does not establish an RTX speedup or guarantee against GPU contention. The queue holds at most one pending frame and drops older pending work, rather than building an increasing backlog. Detection interval limits submissions; it is not an inference FPS claim. To unload the model fully, close the app. Browse again to reload a model replaced at the same path.

If boxes still update slowly, check **Backend**. `ONNX CPU` means this model is not running on the RTX 3070, even if a previous math-model test passed in another app folder. **Copy Detection diagnostics** now includes actual capture FPS, UI frame time, model processing time and result age so these can be distinguished. The updater preserves GPU files in the selected app folder. If diagnostics show that its GPU runtime files are missing, the update includes an optional **GPU-Reinstall/Reinstall-GPU.cmd** helper: leave the demo open and run that helper from the extracted update. It targets the currently running copy, installs pinned local dependencies (up to 1.4 GB; cached downloads reused), then reopens it. It does not run automatically, and installation alone does not verify CUDA execution. A GPU initialization/model error requires diagnosing that actual error, not assuming reinstall is the solution.

## If the combined launcher did not reopen the app

Use the current **Update-And-Enable-GPU.cmd** from a fully extracted update. It targets the selected app explicitly, installs GPU files without opening an intermediate copy, then updates and launches it once. It verifies that the actual app window stays running for 1.5 seconds before reporting success.

If your demo can open, leave only that copy open. If it is already closed, the launcher reuses a previously remembered target or asks you to choose the installed **apex_visualizer.exe**. Select your usual app, not the update's payload or a backup. No manual PowerShell command is required. After target selection it remembers that path for a retry.

If startup fails, the launcher reports the exit/failure and saves **launch-failure.txt** beside that executable, including recent own-app logs and any accessible matching Windows crash record. Send the error text or that file. The original reported reopen failure did not include an exit code, so its exact cause remains unknown; successful launch checks here do not prove this PC has been repaired or guarantee a later model/driver run.

## Roblox test profile (Apex stays primary)

1. Capture Roblox in OBS, fit the full game view to its canvas and start Virtual Camera. In the app use **Capture > Start OBS Virtual Camera**.
2. Open **Detection > Model profile > Roblox (test, YOLO 640px)**.
3. **Choose ONNX model** and choose your local **RobloxAIO-harlan.onnx**. Trained weights are separate and are not included in the public download.
4. Enable **Enable detection**. Leave **Exact analyzed frame** unchecked for live preview; optionally enable the desktop overlay for the selected monitor.
5. Check actual Backend and candidates against visible targets. Only a profiled CUDA execution status proves GPU work; CPU fallback remains explicit. Accuracy and Roblox-specific target coverage require footage, not a black-image sanity test.
6. **Save enemy setup** stores the Roblox model path and thresholds separately. Switching profiles also saves the previous setup; it clears old detections and disables analysis until you select a model or enable it again. Select **Apex (primary, 416px)** to return. The app starts on the Apex profile each launch.

Existing Apex settings remain in `enemy-settings.json`; Roblox uses `roblox-enemy-settings.json` under the same local application folder. Legacy Apex files without a profile field still load as Apex. Unknown/mismatched profile settings are rejected before mutating the selected configuration. Model/profile tensor mismatches are shown explicitly; failed models do not reload continuously until you retry or change options.

The inspected Roblox export uses FP32 `images` [1,3,640,640] -> `output0` [1,5,8400], Ultralytics YOLO single-class channel-major `(centerX,centerY,width,height,score)` pixel output. Its metadata labels class 0 **Enemy**. The adapter uses RGB/[0,1]/gray114 centered letterbox preprocessing, inverse coordinate mapping, score filtering and IoU NMS. It examines all 8400 rows for validity, keeps the top 1000 scored candidates before NMS and caps displayed detections at 300. This class name does not establish real enemy/friendly correctness in every Roblox experience.

## Desktop overlay on your main monitor

1. Open **Screen overlay** in the left sidebar.
2. Click **Show on screen**, or check **Show overlay on my screen**. This also clears a previous F8 hide state.
3. Choose **Screen > Primary monitor**, or the monitor where you play. Enable **Draw FOV circle** if you want a circle.
4. The **APEX VISION | SCREEN OVERLAY ON** badge and configured circle appear immediately on the actual desktop, even without a model or capture. Without a selected game window, the circle is centered on the chosen monitor. With a valid selected Apex window, it follows that client center.
5. For actual detection boxes, start OBS Virtual Camera, connect it in **Capture**, load your local model in **Detection**, enable detection, and keep **Draw detected boxes** checked. Boxes require fresh accepted live detections; a recording never supplies desktop boxes.
6. Use borderless/windowed gameplay on the selected monitor. Exclusive fullscreen can cover desktop windows. You can minimize the demo while the screen overlay runs.
7. Press **F8** to hide/show graphics, or use **Show on screen / Hide overlay**. Close the app to remove it. F8 does not disarm tracking; **F9** does.

The overlay is a separate click-through, non-activating Windows window, independent of the in-app preview. It does not intercept game clicks or keyboard focus. A visible badge or FOV circle is a display check, not evidence of enemy detection. When capture stops, the badge and configured circle remain visible but detection boxes clear. Boxes also clear when the last analysis becomes older than one second.

**Fit capture aspect ratio** centers the captured view with letterboxing/pillarboxing. Disable it only if your displayed game stretches the capture to fill the monitor. Detection boxes assume the full capture corresponds to the selected monitor; they do not automatically track a smaller game window, undo OBS crops or calibrate scene transformations. The Tracking FOV instead follows its explicitly selected client geometry.

The app requests Windows capture exclusion for its overlay (Windows 10 2004+/11). If Windows rejects it, the status warns you to use OBS Window Capture. Actual OBS scene/compositor compatibility still needs checking on your PC; use Window Capture if Display Capture includes the overlay.

## Model contract and limits

The supplied file was inspected and checked: FP32 input `in_00000` `[1,3,416,416]`; FP32 output `out_00000` `[1,300,6]`; end-to-end rows `(x1,y1,x2,y2,score,class)`. The adapter uses bilinear RGB NCHW scaling to `[0,1]`, centered aspect-preserving 416px letterboxing with gray 114 padding, restores boxes to original-frame coordinates, clips them and applies score filtering/IoU suppression. This preprocessing follows the observed YOLO-style contract; full training/preprocessing documentation was not supplied. That is the **Apex** profile. The separate **Roblox** contract is documented above; other shapes are rejected with an error.

The model's actual metadata label is `{0: 'item'}`. You described it as an enemy detector; the UI therefore labels results **Enemy?**. Enemy/friendly classification and accuracy have not been independently established. Scores are model outputs, not calibrated accuracy estimates. No weapons/ammo detector was added. No health, distance, skeleton or hidden-player information is invented from the detector output.

This is analysis of externally captured pixels, shown in the app preview or its separate desktop overlay. It does not inject code into Apex, read its memory or bypass anti-cheat. The optional Tracking category described above can emit normal Windows mouse movement after explicit arming. Only visible image content can be analyzed. Existing Windows OCR text tools remain CPU-based.

GPU inference uses the existing pinned local ONNX/CUDA/cuDNN loader, renderer adapter identity, TF32 disabled, one CPU thread and a 1 GiB CUDA allocator limit (not a limit on all process GPU memory). CPU shape/operator work is permitted and honestly reported. Provider initialization or ordinary inference exceptions fall back to CPU; native driver/DLL process crashes cannot be made recoverable with a C++ exception handler.

See **GAME-ESP-VALIDATION.md** for actual tests and limits. The generated test fixture in source/tests is explicitly synthetic and is never presented as the trained detector or bundled into runtime models.
