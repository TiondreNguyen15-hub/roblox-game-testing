# Apex Visualization — standalone Windows data lab

- [Download Windows x64 build](Apex-Visualization-Windows-x64.zip)
- [Download complete C++ source and offline dependencies](Apex-Visualization-Source.zip)
- [Phase 2 performance report and verified results](apex-visualizer/PHASE2-PERFORMANCE-REPORT.md)
- [Hardware counters, benchmark commands and remaining PC checks](apex-visualizer/HARDWARE-VALIDATION.md)
- [Build instructions](apex-visualizer/README.md)
- [Original feature audit and authorized-data requirements](apex-visualizer/FINAL-AUDIT.md)

Extract the Windows ZIP fully and run `apex_visualizer.exe`. This standalone app displays explicit local simulation or your selected authorized JSON file in its own window. The System page adds real optional GPU/VRAM counters, CPU/RAM, frame and provider timing. NVIDIA counter and CUDA driver detection fail gracefully when unavailable; DirectX remains the renderer.

The native build is checked with Visual Studio 2022, six test suites, two DX11 WARP smoke tests and a timed software-rendering benchmark. These checks do not establish RTX 3070 performance or long-duration hardware stability. Use the included `tools/benchmark.ps1` on that PC for reproducible measurements. No game process integration or memory access exists. Earlier Roblox files remain in this repository.
