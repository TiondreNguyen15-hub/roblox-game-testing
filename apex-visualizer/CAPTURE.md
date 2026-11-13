# External gameplay video input

## OBS Virtual Camera

1. Install/start OBS Studio. Add **Display Capture**, or **Window Capture** using its **Windows Graphics Capture** method. Select your gameplay display/window. Do not select the Game Capture hooking source for this pipeline.
2. Start with 1280x720, 30 FPS in OBS video/output settings. Preview the scene in OBS, then click **Start Virtual Camera**. Avoid a recursive capture of this demo's own preview window.
3. Open this application's **Capture** page and choose **Start OBS Virtual Camera**. Actual decoded pixels, dimensions, capture FPS and preview should appear. **Stop capture** releases the source. Stop/start is supported; missing cameras and source errors appear in the UI and log.

This app uses the Windows Media Foundation video-device API and explicitly selects an OBS-named virtual camera. No game process, hooks, protected memory, network game data or anti-cheat interaction are involved. This is a supported Windows video-input API; it is not a claim of publisher endorsement or anti-cheat certification. Respect recording/privacy rules.

## Local recording

Record using OBS Display/Window Capture. If recording MKV, use OBS **File > Remux Recordings** to make an MP4. In the app paste the absolute MP4 path (without enclosing quote marks) into **Local recording path**, then **Open recording**. Windows-installed Media Foundation codecs determine format support. H264 MP4 is the tested format. End-of-file retains the last frame and reports completion; opening a missing/unsupported file reports an error. Recorded playback is paced by source timestamps rather than silently decoding at maximum speed.

## Implementation and limits

`capture.hpp`/`capture.cpp` define a separate source worker delivering the latest immutable RGBA frame. The worker decodes RGB32 and converts BGRA to RGBA; the UI owns one reusable dynamic DX11 texture and only uploads a new sequence. No unbounded frame queue exists. Maximum frame size is 4096x2160; 720p30 is recommended initially. Decoder buffers and two latest CPU frames still incur real memory/CPU costs; measure on your PC before increasing resolution. Luma/activity sampling is bounded and does not detect players.

The worker owns COM/Media Foundation initialization and RAII teardown; camera shutdown unblocks capture on stop. Failures are logged and leave the renderer active. Initialization may still depend on OBS installation, camera permissions, codec availability and the chosen source. Physical OBS camera operation and captured Apex gameplay need validation on your PC. CI tests only an original generated H264 pattern recording, missing-file rejection and stop/restart.

A future authorized vision-data provider can consume `CaptureSnapshot.frame` and its source timestamp/sequence. It must supply the real model's resize, color normalization, tensor layout, class names and output decoding. The bundled MatMul model cannot analyze imagery. There is deliberately no fabricated entity output from frames.

`tests/fixtures/capture-test.mp4` is an original two-second FFmpeg `testsrc2` pattern (320x180, 15fps, H264). It contains no third-party gameplay. FFmpeg is used to generate the test asset only and is not an application dependency.
