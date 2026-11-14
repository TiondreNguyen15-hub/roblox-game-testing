# Game ESP validation

Native source `d2a3fef5b589867dc5e6e1c1e9df2884469f2b02`. [Windows workflow](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43558523408). EXE SHA256 `c3d0af61de9be24cd9c22169ae721aee4d326c2a0a70a6ccc91c7867b1226174`.

| Check | Verified result and scope |
|---|---|
| VS2022 x64 Release | Passed; the native MSVC executable from this workflow is packaged |
| Native module suites | 13/13 passed, including enemy RGB/letterbox geometry, inverse boxes, confidence/IoU filtering and invalid-class rejection |
| Linux module / ASAN+UBSAN suites | 11/11 each passed; no sanitizer failure in these tested paths |
| Actual Windows enemy adapter integration | Passed: original H264 numeric video -> real capture pixels -> image-dependent synthetic ONNX fixture -> one decoded candidate -> actual analyzed-frame DX11 draw command. This tests plumbing, not enemy recognition |
| Existing OCR integration | Passed: original recorded pixels -> actual Windows OCR -> 28/160 numeric candidates -> preview/CSV |
| Existing camera receiver | Passed using actual official OBS 33.0.1 filter idle output on CI, not gameplay |
| Existing stability / compute checks | 120-second software DX11 WARP rendering, actual CPU inference/custom-model checks, model recovery, cancellation and missing dependencies passed their expected contracts |
| Private supplied model | ONNX checker and actual Linux ONNX Runtime 1.22 CPU execution passed. FP32 input [1,3,416,416], output [1,300,6], all finite; class 0; maximum black-image score 0.0000196695, zero candidates at 0.50. Black input is a contract/sanity check, not an accuracy test |
| CUDA enemy-model execution | Not run here: hosted Windows runner has no RTX 3070. App enables its verified status only after an actual non-copy CUDA node appears in its ONNX profile. CPU operator work is allowed and labeled |
| Updater preservation | [Focused Windows test](https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43558633916) passed syntax, SHA integrity, replace/backup, spaced paths and GPU/model/settings preservation in an isolated folder. It did not exercise GUI selection or closing a real app |
| Actual gameplay accuracy / long model soak | Not tested. Requires captured footage, labels and the user's PC. No speedup, detection accuracy or leak-freedom claim |

The private model SHA256 is `e7f89487dc8970e8f6da4e70ed56756d3287b4f3dfef97ce973ef7138be280bc` (40,793,716 bytes). Its metadata is `{0: 'item'}`; the user's enemy-only meaning is shown as **Enemy?**, not independently verified friend/foe classification. Preprocessing uses RGB/[0,1]/gray114 letterboxing inferred from the observed model contract; training documentation and labeled footage should confirm it. No private weights or access link are published. Synthetic test weights stay in source/tests, outside runtime models.

The first Windows integration run caught a layout regression: new enemy controls pushed the preserved OCR preview below the visible page. Collapsible sections now keep the selected tools visible; the final native workflow above retested both previews.

New modules: enemy.hpp, enemy.cpp (image preparation/box decoding/settings), enemy_service.cpp (bounded latest-frame worker), enemy_inference.inc (existing-loader ONNX CPU/CUDA adapter). Menu/main integrate local file selection, analyzed-frame textures, boxes, thresholds, diagnostics and save/load. The old HUD Reader sidebar label becomes Game ESP; existing text tools remain collapsed beneath it. No weapons/ammo model, game overlay, memory access or automation was added. Existing local settings and installed runtimes remain.

Raw records are in [validation/game-esp](validation/game-esp), including the private-model CPU contract check (no weights), actual native fixture output, workflow log, sanitizer records and updater log. See [usage](GAME-ESP-README.md).
