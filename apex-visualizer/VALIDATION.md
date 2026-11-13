# Final validation record

## Native Windows validation passed

Run: https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43359752528
Source commit: e6330e0

- Visual Studio 2022 x64 Release compilation succeeded, with static C/C++ runtime.
- All five CTest suites passed: core/config, actual ImGui visuals, menu/system, providers/JSON/async handoff, and logging.
- DX11 WARP synchronous smoke: 6 frames, 2 file entities, flip presentation, 3,400 vertices and 5 final-frame draw calls.
- DX11 WARP asynchronous smoke: 60 frames, 2 file entities, flip presentation, 3,400 vertices and 5 final-frame draw calls.
- Async GPU timestamps produced valid measurements on the Microsoft Basic Render Driver. These are WARP measurements, **not RTX 3070 benchmarks**.
- The final runtime ZIP uses the actual native MSVC executable from this successful CI artifact. It is not the earlier cross-build.
- Import inspection shows only Windows system DLLs, without MSVCP/VCRUNTIME redistribution dependencies.

## Local validation passed

All five native C++17 test suites passed. Windows cross-compilation/linking also passed. AddressSanitizer and UndefinedBehaviorSanitizer tests passed for the JSON/provider path, including source switching and the background-worker handoff. The unused ImGui demo source is excluded from the offline dependency bundle.

Input checks cover exact sample compatibility, shields, position/distance updates, missing/partial/oversized JSON, stale data, last-valid retention, source switching, async result handoff, missing/supplied bones and unchanged settings. Visual tests generate real ImGui draw data. Logging tests cover sanitization, rotation and unwritable paths.

## Remaining external / physical checks

No authorized live Apex dataset is supplied by this project. Only explicit local simulated data is bundled. Actual external entity fields, bones, visibility, teams, distances and coordinates must be supplied by an authorized producer using the documented schema.

RTX 3070 hardware was unavailable on the build host. Validate physical GPU selection, FPS, GPU draw time, DPI, resize/minimize/restore, driver resets and a ten-minute memory run on that PC. No hardware-specific frame rate, overall GPU utilization percentage or lag-compensation claim is made. Device removal reports a logged error and exits cleanly; restart is required.

See FINAL-AUDIT.md for all implemented changes, resource-lifetime policy and remaining acceptance checks. preview.png is a CPU rasterization of real ImGui geometry, not a Windows hardware screenshot.
