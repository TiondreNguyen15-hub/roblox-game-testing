# OBS Virtual Camera compatibility update

The Phase 3 camera button originally searched only Windows Media Foundation. OBS Virtual Camera uses a DirectShow video-input filter on Windows, so a registered OBS camera may not appear in that enumeration. Starting the camera alone cannot solve that backend mismatch. This is a likely explanation for the reported "OBS Virtual Camera not found" message; the user's actual device enumeration still needs checking.

The updated application keeps Media Foundation recording/camera support and adds a DirectShow fallback when that API finds no OBS camera. The fallback enumerates the 64-bit DirectShow video-input category, selects only an OBS-named virtual camera, connects its actual video stream through Windows RGB32 conversion and a sample callback, converts to RGBA and feeds the existing DX11 preview. FPS counts delivered real samples, not polling ticks. Latest-frame storage is bounded, missing/invalid/stalled streams are errors, and stopping releases the graph and camera.

## Update

1. Close the demo.
2. Extract `Apex-Visualization-OBS-Fix.zip` into your **existing folder containing apex_visualizer.exe**, replacing the EXE. Keep your `runtime/gpu` and settings.
3. In OBS, use Display Capture or Window Capture with Windows Graphics Capture. Start Virtual Camera.
4. Launch the updated demo and choose Capture > Start OBS Virtual Camera. The source label should indicate DirectShow if the fallback was needed.
5. If capture still fails, click **Copy capture diagnostics** and paste the text in chat. The error now distinguishes an unregistered camera from a graph/open/format failure. The log also lists camera names returned by DirectShow; it does not inspect game memory or capture physical cameras automatically.

If neither API finds OBS, repair/reinstall the current official **64-bit OBS Studio** installation and restart the demo. If the error reports access denied, check Windows camera permissions for desktop apps. These are conditional troubleshooting steps, not evidence that either is the cause on this PC.

## Verification scope

The new native Windows build must pass real DirectShow video decoding through its sample callback, existing MP4 capture and DX11 preview checks. An original uncompressed AVI test pattern is used to exercise the graph/callback without installing or impersonating an OBS camera. Enumeration/missing-camera errors are tested separately. Actual OBS camera acquisition on the user's PC remains unverified until the updated application receives frames there.

No renderer, CUDA/ONNX code, settings format, JSON provider or working main-menu pages were replaced. No injection or anti-cheat bypass is involved. Semantic gameplay inference still requires an appropriate authorized vision model.
