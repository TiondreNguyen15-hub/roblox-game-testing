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
