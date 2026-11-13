> Current compute phase: [GPU-COMPUTE.md](GPU-COMPUTE.md) and [GPU-COMPUTE-REPORT.md](GPU-COMPUTE-REPORT.md) supersede earlier detection-only/no-model descriptions. This record preserves the previous phase's measurements.

> Phase 2 adds hardware telemetry and benchmarking; see [HARDWARE-VALIDATION.md](HARDWARE-VALIDATION.md) and [PHASE2-PERFORMANCE-REPORT.md](PHASE2-PERFORMANCE-REPORT.md) for current behavior and validation. The record below describes the preceding audit.

# Final application audit

## Completed implementation

### Providers and JSON

- Polymorphic provider selection through `IDataSource` and `ProviderSystem`, with explicit simulation/file modes.
- File reads, validation and parsing run on one background worker in the interactive application. A bounded single pending request/result prevents unbounded queues. Source/path generations discard obsolete results when switching modes or paths.
- The existing `players` schema remains compatible. Names, health, shields, supplied distance, screen-space boxes, visibility and optional teams update from accepted snapshots.
- Optional explicit 15-joint screen-space skeletons are supported. Versioned 3D records accept supplied bones and shields. Missing bones are no longer replaced with a fabricated pose. Only the clearly selected simulator generates entities/poses.
- 64-entity / 256 KiB / 16-level bounds; type/range/name checks; immutable last-valid snapshot on error; ancient-file timestamp overflow regression fixed; empty-array clearing; stale markers; path switching; stable-file checks; no reparsing of unchanged valid contents.
- Startup can find the bundled sample beside the executable when the working directory differs. `--json` selects an explicit authorized file.

### Visual correctness and UI preservation

The same sidebar, tabs, hotkeys, appearance settings, filters, configuration location and version-1 settings remain. Copy/Paste/context actions and quoted paths remain supported. Health and shield numeric/bar controls no longer depend on one another. Shield bars also work for 3D records. A skeleton is drawn only when bone data is supplied or the simulator is selected. Unknown teams are not invented. The file status no longer paints over menu controls.

### Rendering / RTX 3070 readiness

- Prefer a non-software adapter with higher dedicated VRAM instead of inadvertently defaulting to an integrated GPU. Actual adapter name/memory remain visible; `--warp` explicitly selects software rendering.
- Modern DXGI flip-discard presentation with two buffers, one-frame latency limit and a waitable frame signal; compatibility swap-chain fallback is retained.
- Four asynchronous timestamp/disjoint query sets measure actual GPU draw duration without a blocking GPU readback. Disjoint/unsupported samples are ignored. The System tab displays measured draw time and active presentation path.
- Existing VSync, frame cap, minimized/occluded throttling, CPU entity filtering and batched ImGui rendering are retained.
- No RTX 3070-specific FPS or utilization number is fabricated. This environment does not contain the user's RTX 3070; see the hardware acceptance procedure below.

### Logging, errors, dependencies and launch package

- UTC diagnostic log: `%LOCALAPPDATA%\ApexVisualization\apex.log`; rotation keeps a bounded current log and one backup. It records startup/shutdown, actual renderer selection, configuration results, provider errors/recovery and fatal errors. Repeated identical provider errors are not logged every frame.
- Logging failures do not terminate recovery. Existing configuration files are loaded without format changes; writes remain atomic.
- DX11 device, swap-chain, query, wait-handle, ImGui and Win32 lifetimes have explicit cleanup. Shader/font creation is checked. Device/present failures report an error and exit cleanly; automatic device recreation is deliberately not implemented.
- Only Dear ImGui, nlohmann/json and Windows SDK/system facilities are required. The unused upstream ImGui demo source is excluded from the offline ZIP. The runtime package contains the app/data/docs/licenses; test executables remain CI artifacts/source builds.
- MSVC uses the static C/C++ runtime, avoiding a separate Visual C++ Redistributable requirement for the final native build.

## Audit tests

The local suite covers configuration compatibility, quoted paths, bounds, parsing, missing/partial/oversized input, last-valid recovery, stale data, actual ImGui geometry, absent/supplied skeleton behavior, independent shield controls, source switching, async-reader handoff, logging rotation and unwritable log destinations. All five suites passed locally and on Windows. Address/undefined-behavior sanitizer checks passed. Native Windows DX11 synchronous and asynchronous smoke tests passed. Exact evidence is recorded in VALIDATION.md.

Windows CI runs both the six-frame synchronous smoke test and a 60-frame asynchronous-reader smoke test against the bundled JSON. The interactive app uses the background reader; worker switching and cleanup are also unit tested.

## Authorized external input required

The application does not obtain real Apex player data. To display anything beyond the provided simulated sample, an authorized producer must write the documented JSON file. Real health, shield, visibility, teams, distances, screen coordinates and bones must come from that producer. No missing field is discovered by reading a game process. Actual 3D data must use the documented fixed camera/coordinate convention; arbitrary game camera matrices are not inferred.

## Remaining hardware acceptance checks

On the RTX 3070 PC, verify that the System tab identifies the expected adapter and shows flip-discard presentation. Test 64 entities, update the local file continuously, compare VSync/frame caps, and record measured FPS, CPU work and GPU draw duration. Check 100/150/200% DPI, resize/minimize/restore and a ten-minute memory run. These physical-device checks cannot be certified remotely without results from that hardware.

The application is launchable and its supported data path is implemented. A changing game dataset still requires an authorized external producer. Device removal requires restarting after the logged error. Network filesystems are not a performance target; use a local file. Shutting down waits for any in-flight local file read to finish.
