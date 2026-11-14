# OBS camera detected, but no frames: startup update

The user reported that DirectShow found and opened OBS Virtual Camera, but received zero callbacks before the eight-second timeout. That proves discovery/graph connection progressed; it does not identify the cause of missing samples or prove OBS output was working.

The app now explicitly prepares the graph with `IMediaControl::Pause()` before `Run()`. [OBS 33.0.1 VCamFilter source](https://github.com/obsproject/obs-studio/blob/33.0.1/plugins/win-dshow/virtualcam-module/virtualcam-filter.cpp) signals its capture thread's start event in `Pause()`, while its `Run()` only marks it active and delegates to the base filter. Making the live-source state transition explicit avoids relying on implicit preparation. This is a compatibility change, not a remotely proven root cause on the user's PC.

Graph failure/device-loss events are now consumed and reported. Startup HRESULTs and graph state are logged. A no-sample timeout includes callback count and Pause/Run/state results rather than assuming OBS was not started.

## Install

Close the app. Extract the latest `Apex-Visualization-OBS-Fix.zip` into the **existing folder with apex_visualizer.exe**, replacing the executable. Keep `runtime/gpu` and settings. In OBS start Virtual Camera, then reopen the demo and select Capture > Start OBS Virtual Camera.

If it still fails, choose Copy capture diagnostics and paste the text in chat. Also check whether OBS Virtual Camera produces video in another camera-capable app, such as a browser video preview or video-call application's camera settings. That distinguishes an OBS output problem from this app's receiver; it is a diagnostic check, not a requirement to reinstall OBS.

## Actual test and limits

The new Windows workflow downloads the pinned official OBS Studio 33.0.1 x64 archive, verifies its published SHA256, registers its actual 64-bit virtual-camera component in the isolated CI machine, and runs this application's camera-to-DX11 preview test. It does not register a fake camera or start OBS/gameplay. With no OBS producer, the real filter's idle/placeholder frames are expected; those prove actual camera sample delivery and preview, not gameplay capture. The component is unregistered afterward and is not redistributed in this patch.

A passing workflow is recorded separately before native packaging. An actual user camera, OBS producer output and gameplay still require confirmation on that PC. Existing AVI/MP4 tests, renderer, CPU/ONNX checks and settings are retained. No GPU inference speedup or anti-cheat certification is asserted.
