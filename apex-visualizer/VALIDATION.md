# Validation record

## Passed

- Native C++17 build with GNU C++ 14.2; all three CTest suites passed.
- AddressSanitizer and UndefinedBehaviorSanitizer core tests passed.
- Windows x64 cross-build with Clang 18.1.6/Zig 0.13.0; application and all three test executables linked.
- **Native Visual Studio 2022 Windows CI build passed.**
- **All three test suites passed on Windows.**
- **Real DX11 WARP smoke test passed:** six frames, Microsoft Basic Render Driver, 5 draw calls and 5,840 vertices in the final frame.
- Dependency archive hash and delivery ZIP integrity verified.

Windows run: https://github.com/TiondreNguyen15-hub/roblox-game-testing/actions/runs/43354535661

## Scope

Core tests cover settings bounds, config round-trips/overwrites, malformed/unsupported/non-finite/oversized input rejection, unchanged state on failed load, projection and simulation invariants. Visual tests generate real ImGui draw lists with filters and corner/overlay toggles. Menu tests render all four pages and fade frames and check process CPU/uptime. The WARP smoke test creates a real Win32/DX11 window and exercises the ImGui DX11 backend.

preview.png is CPU rasterization of actual ImGui geometry, not a Windows GPU screenshot. The source archive vendors Dear ImGui v1.91.9b and its MIT license.

The direct Windows ZIP contains the Clang/Zig cross-built executables. The same source passed the native MSVC/WARP checks above; MSVC-generated executable artifacts are also available from that Actions run. The cross-built ZIP itself has not been launched on a physical PC here.

## Remaining manual checks

- Physical Windows 10/11 hardware GPUs, monitor refresh rates and long-run memory/FPS behavior.
- Insert/F10 and all control interactions, config permission failures and malformed-file warning UI.
- Resize, minimize/restore, occlusion and 100/150/200% DPI across real monitors.
- Hardware-specific device removal/failure behavior. The app reports an HRESULT and releases resources on device loss; it does not recreate the device automatically.

No universal FPS guarantee, GPU utilization or GPU frame-time measurement is claimed. Process CPU is normalized over logical cores, not whole-machine CPU usage. All scene entities and visibility are generated within the application.

## Delivery

Project, complete source archive, cross-built Windows archive, checksum manifest and Windows CI workflow are uploaded in the existing repository. Earlier Roblox files remain preserved.
