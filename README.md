# Recorded HUD text reader

[Download the one-click HUD update](Apex-HUD-Update.zip) · [Usage](apex-visualizer/HUD-README.md) · [Actual OCR/native validation](apex-visualizer/HUD-VALIDATION.md)

Leave your working demo open, extract **all** files from the HUD update ZIP, and double-click **Update-HUD.cmd**. It verifies and installs the native executable in the current app folder, backs up the old EXE, and keeps GPU libraries, existing models and settings. No GPU reinstall is required.

Open **HUD Reader > Open recording...**, choose your MP4, pause it, drag a box around the magazine digits, and select **Read current frame** or continuous reading. `HUD-Sample.mp4` is included in the app folder for a first test (use Text upscale 1). Save HUD setup stores the region/mode separately; CSV export keeps the latest 600 observations for the selected source.

This is actual **Windows OCR on CPU**, separate from CUDA model validation. Raw text, optional numeric/banner candidates, frame IDs/timestamps and measured OCR latency are shown/exported. No confidence score, Apex-trained object detector, live-player data, automatic aiming or gameplay accuracy is fabricated. Actual Apex footage still needs region calibration and accuracy checks. Existing OBS capture remains; this OCR phase analyzes local recordings.

Native Windows tests passed 12/12, including actual numeric pixel OCR and a full H264 recording -> OCR -> parsed 28/160 -> DX11 preview/CSV test using an original generated fixture. Portable/sanitizer suites passed 10/10. The updater's Windows preservation test also passed; its GUI picker/real app closing were not exercised by that isolated test.

---

# GPU runtime reinstall without folder searches

[Download the double-click GPU reinstall launcher](Apex-GPU-Reinstall.zip)

Leave your current Apex Visualization demo open, extract **all** files from this ZIP, and double-click `GPU-Reinstall/Reinstall-GPU.cmd`. It detects the running demo, closes it normally, installs the version-pinned GPU libraries into that app folder, and reopens the demo. If no unique running demo is found, it uses the last known Compute-Fix folder or provides an EXE picker.

About 1.4 GB may download; cached packages are reused and hashes verified. No CUDA toolkit or system PATH changes are made. Existing GPU files are backed up. After installation, use System > Run CUDA + ONNX validation. DLL installation is not proof of GPU execution. [Launcher validation scope](tools/gpu-reinstall/VALIDATION.txt).

---

# Five-minute GPU test startup repair

[Download the compute startup fix](Apex-Visualization-Compute-Fix.zip) · [Instructions](apex-visualizer/COMPUTE-STARTUP-FIX.md) · [Actual validation](apex-visualizer/COMPUTE-STARTUP-VALIDATION.md)

This update includes the missing mathematical test model and CPU runtime, recovers a missing default model, accepts quoted Windows paths, adds **Use bundled test model**, and exposes the actual startup exception in the UI and copied diagnostics. Existing custom files, GPU libraries, OBS capture, settings and DX11 rendering remain.

Close the app and extract into your **original full app folder containing runtime/gpu**, replacing the executable and supplied files. Keep all installed GPU DLLs. Launch > System > Use bundled test model > 5-minute GPU model test. The patch does not include or reinstall the 1.41 GiB GPU dependencies.

Native Windows tests verified recovered-model bytes/SHA256, actual CPU inference through the soak startup path, honest missing-file errors, and existing camera/rendering/inference regressions. The exact original PC exception was not supplied; actual RTX 3070 five-minute execution still needs a retry on that PC. Copy compute diagnostics now includes any remaining error.

---

# OBS camera no-frame startup fix

[Download the updated OBS patch](Apex-Visualization-OBS-Fix.zip) · [Startup fix details](apex-visualizer/OBS-STARTUP-FIX.md) · [Actual camera validation](apex-visualizer/OBS-CAPTURE-VALIDATION.md)

The receiver now connects native camera pins explicitly, configures YUY2 (including fresh cameras with no saved format), converts frames to RGBA internally, prepares the live source and removes playback-clock scheduling from live capture. A real OBS camera test reproduced a one-frame stall with state Running; the updated app passed with ten actual official OBS 33.0.1 camera samples and a DX11 preview draw command. These were idle/placeholder frames in isolated Windows CI, not captured gameplay or a test on your PC.

Close the app, extract the patch into your **existing folder containing apex_visualizer.exe**, replace the EXE and keep `runtime/gpu` and settings. Start OBS Virtual Camera, reopen the updated app and retry Capture. If it fails, use Copy capture diagnostics and paste the text in chat.

---

# OBS Virtual Camera detection update

[Download the small OBS compatibility patch](Apex-Visualization-OBS-Fix.zip) · [Actual native Windows validation](apex-visualizer/OBS-CAPTURE-VALIDATION.md)

The original camera button searched only Media Foundation. The updated app also tries DirectShow, which OBS Virtual Camera commonly uses. Close the app, extract this patch into your **existing folder containing apex_visualizer.exe**, and replace the EXE. Keep your installed `runtime/gpu` and settings.

Start Virtual Camera in OBS, then Capture > Start OBS Virtual Camera in the updated app. If it still fails, choose **Copy capture diagnostics** and paste that text in chat. DirectShow video decoding and existing renderer/CPU regression tests passed; actual OBS acquisition on your PC still needs confirmation. [Instructions and limits](apex-visualizer/OBS-CAPTURE-FIX.md).

---

# Apex Visualization — Phase 3 performance and external capture

- [Small Phase 3 update — keep your installed GPU runtime](Apex-Visualization-Phase3-Update.zip)
- [Full Windows x64 application](Apex-Visualization-Windows-x64.zip)
- [Complete C++ source and offline build dependencies](Apex-Visualization-Source.zip)
- [Benchmark and five-minute GPU test instructions](apex-visualizer/PHASE3-PERFORMANCE.md)
- [OBS / local recording capture guide](apex-visualizer/CAPTURE.md)
- [Actual build/test evidence and remaining PC checks](apex-visualizer/PHASE3-VALIDATION.md)

Close the application. Extract the small update into the **existing folder containing `apex_visualizer.exe`**, replace the EXE, and keep your working `runtime/gpu` folder. No GPU dependency reinstall is needed for an already-working installation. Fresh full downloads contain the CPU runtime; the optional pinned GPU setup helper remains available.

In **System**, leave Model path blank for the bundled mathematical validation model, or select your own supported FP32 ONNX file. Choose **Benchmark loaded model CPU / GPU** or **5-minute GPU model test**. The app reports actual Run latency, validated-loop inference FPS, GPU utilization, VRAM and memory trends. A completed short CUDA test does not establish five-minute stability or a speedup. Current RTX 3070 timings must be measured on your PC; none are fabricated.

The **Capture** page receives real frames from OBS Virtual Camera or a local recording using Windows Media Foundation. Use OBS Display Capture / Window Capture with Windows Graphics Capture. This app does not inject, access another process's memory, bypass anti-cheat or invent player entities. Preview and frame activity work on pixels; semantic gameplay detection still requires a compatible authorized vision model and its preprocessing/output contract. The bundled MatMul model is not a vision detector.

The existing interface, settings, DX11 renderer, local JSON input and earlier project assets are preserved. Historical evidence remains available; the Phase 3 validation report describes this native build specifically.
