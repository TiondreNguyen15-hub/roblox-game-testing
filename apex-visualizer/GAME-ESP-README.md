# Game ESP: local enemy-model capture analysis

The sidebar's **HUD Reader** entry is now **Game ESP**. This page loads your local trained model and draws candidate boxes on the exact captured frame that was analyzed. Existing recording text tools remain in a collapsed section; JSON/simulated visuals, settings, OBS capture, DX11 rendering and compute benchmarks remain.

## Install and start

1. Leave your current working demo open. Download **Apex-Game-ESP-Update.zip**, right-click it and choose **Extract All**. Open the extracted folder and double-click **Update-Game-ESP.cmd**. The updater locates your running demo, closes it normally, backs up its executable, updates it and reopens it. Keep your existing GPU libraries; no GPU reinstall is required.
2. Open **Game ESP**, then **Browse enemy ONNX model**. Select your local `SingularityMetaV3_recovered.onnx` file. If it is inside a ZIP, extract the ZIP first. The private trained weights are deliberately not included in the public download.
3. Choose **Open recording for enemy analysis**, or start your existing OBS receiver on **Capture** and return to **Game ESP**. Browse automatically enables incoming-frame analysis. **Detect enemies in current frame** also works with a paused recording.
4. Adjust **Detection score** and **Overlap suppression** if needed. Default score is 0.50. **Save enemy setup** preserves the local model path, CUDA preference, thresholds and analysis interval in `%LOCALAPPDATA%/ApexVisualization/enemy-settings.json`.
5. Read **Backend**. Only `CUDA EP execution profiled (CPU ops permitted)` means this detector actually executed a non-copy CUDA node. It is separate from the math-model validation. CPU fallback includes its reason. Use **Copy Game ESP diagnostics** if initialization fails.

The analyzed frame can lag the live capture; its source sequence and total processing latency are shown. The queue holds at most one pending frame and drops older pending work, rather than building an increasing backlog. Detection interval limits submissions; it is not an inference FPS claim. To unload the model fully, close the app. Browse again to reload a model replaced at the same path.

## Model contract and limits

The supplied file was inspected and checked: FP32 input `in_00000` `[1,3,416,416]`; FP32 output `out_00000` `[1,300,6]`; end-to-end rows `(x1,y1,x2,y2,score,class)`. The adapter uses bilinear RGB NCHW scaling to `[0,1]`, centered aspect-preserving 416px letterboxing with gray 114 padding, restores boxes to original-frame coordinates, clips them and applies score filtering/IoU suppression. This preprocessing follows the observed YOLO-style contract; full training/preprocessing documentation was not supplied. Other model shapes are rejected with an error.

The model's actual metadata label is `{0: 'item'}`. You described it as an enemy detector; the UI therefore labels results **Enemy?**. Enemy/friendly classification and accuracy have not been independently established. Scores are model outputs, not calibrated accuracy estimates. No weapons/ammo detector was added. No health, distance, skeleton or hidden-player information is invented from the detector output.

This is analysis of externally captured pixels inside the standalone app. It does not add an in-game overlay, inject into Apex, read its memory, bypass anti-cheat or automate gameplay. Only visible image content can be analyzed. Existing Windows OCR text tools remain CPU-based.

GPU inference uses the existing pinned local ONNX/CUDA/cuDNN loader, renderer adapter identity, TF32 disabled, one CPU thread and a 1 GiB CUDA allocator limit (not a limit on all process GPU memory). CPU shape/operator work is permitted and honestly reported. Provider initialization or ordinary inference exceptions fall back to CPU; native driver/DLL process crashes cannot be made recoverable with a C++ exception handler.

See **GAME-ESP-VALIDATION.md** for actual tests and limits. The generated test fixture in source/tests is explicitly synthetic and is never presented as the trained detector or bundled into runtime models.
