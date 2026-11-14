# OBS camera detected, but no frames: startup update

The user reported that DirectShow found and opened OBS Virtual Camera, but received zero callbacks before the eight-second timeout. That proves discovery/graph connection progressed; it does not identify the cause of missing samples or prove OBS output was working.

Real-filter testing also exposed failures in the capture graph builder's automatic connection/interface-discovery path. The receiver now enumerates OBS's actual output pin, obtains IAMStreamConfig directly, selects a supported native YUY2 format, and uses ConnectDirect through the sample receiver to the null renderer. Camera video no longer depends on a guessed RGB conversion filter. If a fresh OBS filter advertises zero formats because no producer has cached a resolution, the app requests a concrete 1280x720, 30fps YUY2 format through IAMStreamConfig instead of rejecting the empty list. A bounded in-app YUY2-to-RGBA conversion has tested black/white/red reference cases and rejects malformed dimensions. Its color matrix is BT.601 limited-range; a vision preprocessing contract must account for source color characteristics.

The app now explicitly prepares the graph with `IMediaControl::Pause()` before `Run()`. [OBS 33.0.1 VCamFilter source](https://github.com/obsproject/obs-studio/blob/33.0.1/plugins/win-dshow/virtualcam-module/virtualcam-filter.cpp) signals its capture thread's start event in `Pause()`, while its `Run()` only marks it active and delegates to the base filter. Making the live-source state transition explicit avoids relying on implicit preparation. This is a compatibility change, not a remotely proven root cause on the user's PC.

A real-camera test then reproduced a one-frame stall while the graph reported Running. The live path now removes playback-clock synchronization from the graph: OBS paces its own frames, and the null renderer consumes them without waiting on incompatible playback timestamps. File-recording playback keeps its existing clock.

Graph failure/device-loss events are now consumed and reported. Startup HRESULTs and graph state are logged. A no-sample timeout includes callback count and Pause/Run/state results rather than assuming OBS was not started.

## Install

Close the app. Extract the latest `Apex-Visualization-OBS-Fix.zip` into the **existing folder with apex_visualizer.exe**, replacing the executable. Keep `runtime/gpu` and settings. In OBS start Virtual Camera, then reopen the demo and select Capture > Start OBS Virtual Camera.

If it still fails, choose Copy capture diagnostics and paste the text in chat. Also check whether OBS Virtual Camera produces video in another camera-capable app, such as a browser video preview or video-call application's camera settings. That distinguishes an OBS output problem from this app's receiver; it is a diagnostic check, not a requirement to reinstall OBS.

## Actual test and limits

The new Windows workflow downloads the pinned official OBS Studio 33.0.1 x64 archive, verifies its published SHA256, registers its actual 64-bit virtual-camera component in the isolated CI machine, and runs this application's camera-to-DX11 preview test. It does not register a fake camera or start OBS/gameplay. With no OBS producer, the real filter's idle/placeholder frames are expected; those prove actual camera sample delivery and preview, not gameplay capture. The component is unregistered afterward and is not redistributed in this patch.

A passing workflow is recorded separately before native packaging. An actual user camera, OBS producer output and gameplay still require confirmation on that PC. Existing AVI/MP4 tests, renderer, CPU/ONNX checks and settings are retained. No GPU inference speedup or anti-cheat certification is asserted.
