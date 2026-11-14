# HUD OCR validation

Native source `79fed8a5b8b9d1985e6cc6198ce9185c422d5133`. [Actual native Windows workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43553817653). EXE SHA256 `c9be18e94c6f6f7ff3acbcd1e7fbc396f5479ff841ad14fbc7429bc57a68b5c3`.

| Check | Actual result / limit |
|---|---|
| Visual Studio 2022 x64 Release | Passed; this native executable is packaged |
| Native CTest | 12/12 passed, including actual Windows OCR on generated numeric pixels |
| Portable / sanitizer tests | 10/10 each passed; parser, crop mapping, scale, save/load preservation and CSV escaping included |
| Actual static OCR | Native engine read `28 / 160` from a GDI-rendered image; worker and CSV delivery also checked |
| MP4 -> capture -> region -> OCR -> DX11 | Passed: the real H264 decoder, crop, OCR engine, parsed counters and preview draw command were exercised |
| Video OCR observations | The original fixture produced four actual exported readings of magazine 28 / reserve 160; CSV contains frame IDs, source timestamps and measured CPU latencies |
| Recording pause/resume | Passed: captured sequence remains unchanged while paused, resumes afterward, then closes cleanly |
| OBS virtual camera | Existing actual official OBS 33.0.1 idle/placeholder frame -> DX11 test passed; no gameplay producer was claimed on CI |
| Renderer and inference regression | Native smoke tests, 120-second WARP rendering, actual CPU ONNX/custom-model checks, model recovery, cancellation and missing dependencies passed their contracts |
| Updater preservation | [Windows updater workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43554516657) parsed the script and tested SHA validation, replace/backup, spaced paths and preserving GPU/model/settings files in an isolated folder. GUI selection and closing a real app were not exercised by that test |
| Actual Apex footage / HUD accuracy | Not tested: user footage and region calibration required |
| GPU OCR | Not implemented or claimed; this text backend is Windows OCR on CPU |

No FPS improvement, RTX-specific OCR performance, recognition confidence score, protected game data or simulated player detection is asserted. Native video text and its numeric candidates come from actual pixels of an original generated test fixture. This is not a benchmark or accuracy test on the user's PC.

The first native test iteration found an open test CSV handle preventing Windows cleanup; that test was corrected. The final workflow above passed all checks. Existing UI/settings, local JSON input, DX11 rendering, CUDA validation and camera receiver remain.

New modules: hud.hpp, hud.cpp (regions/parser/settings/CSV), hud_service.cpp (bounded worker), hud_ocr_winrt.cpp (actual Windows text recognizer), and explicit non-native-platform unavailable backend. Capture gained recorded-playback pause; the menu gained a sixth HUD Reader page and recording/CSV file pickers. The sample MP4 was generated locally with FFmpeg drawtext and is not actual gameplay.

Raw records: [validation/windows-hud-ocr](validation/windows-hud-ocr), including hud-smoke-result.txt, hud-readings-test.csv, native workflow log and updater log. [Usage and limitations](HUD-README.md).
