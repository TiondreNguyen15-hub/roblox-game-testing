# Local JSON input

1. Extract the Windows ZIP.
2. Keep **apex_entities.json** beside the executable (or enter its absolute path).
3. Run **apex_visualizer.exe**. Press **Insert**, open **Visuals**, and check **Read local JSON file** (enabled by default).
4. Enter the file path if necessary. Press Insert again to see the full visualization.
5. Edit/save the JSON; the app checks for updates five times per second.

The included apex_entities.json is exactly the two-player simulated example. This source is a local file supplied by your own tool. No process memory, game client, network service or anti-cheat is accessed.

```json
{
  "players": [
    {"name":"Player 1","health":100,"shield":100,"distance":45,"x":500,"y":300,"visible":true},
    {"name":"Player 2","health":75,"shield":50,"distance":90,"x":800,"y":400,"visible":true}
  ]
}
```

## Coordinate contract

- x/y are the **top-left box position in client-area pixels** of this app, not world coordinates or coordinates from another program.
- Default width/height: 48×100 pixels. Optional numeric `width` and `height` override these.
- Health and shield are numeric percentages, 0–100. Health is a green/red side bar; shield is a blue bottom bar. Numeric HP/SH labels are also drawn.
- Distance is supplied by your tool in meters, used by the distance label and maximum-distance filter.
- visible must be a JSON boolean; it controls the existing visibility label/color/filter.
- Optional integer team is 0–63; if omitted, no team identifier is fabricated or displayed. Explicit teams are zero-based. No team is inferred from a game.
- Names must be 1–64 UTF-8 bytes without control characters.
- This format supplies no bone positions, so **skeleton lines are not fabricated** for it. The ordinary simulation still supports skeletons. A separate optional schema-1 3D format is supported by the data module for future authorized development data.

At most 64 players and 256 KiB per snapshot; nesting is limited to 16. Invalid/missing fields, invalid numbers and malformed JSON are rejected. A valid empty players array clears the scene. An error retains the last valid frame with a visible FILE ERROR status. Changing the path clears data from the previous source. A file not updated for two seconds is marked STALE; rereading it does not fabricate freshness.

Use a temporary file in the same directory followed by atomic replacement. Readers can observe partial writes from non-atomic producers; they recover on the next valid update. The included writer retries transient Windows sharing errors.

## Simulated writer

With Python installed, run from the extracted source folder:

```
python examples/write_entities.py --output "C:\path\to\apex_entities.json"
```

It generates its own moving sample records. It does not read another application's entities. Ctrl+C stops it.

## Architecture and persistence

`IDataSource` defines the read-only entity/status contract; `Telemetry` implements bounded local JSON polling/validation; `Visuals` accepts a vector of entities independent from transport. A future authorized source can provide the same Entity records without changing drawing code. `Simulation` remains available by disabling the file-source checkbox.

The source mode and path save/load with the existing configuration buttons. Relative paths resolve against the current working directory; an absolute path is most reliable when launching from different shells. nlohmann/json v3.11.3 is vendored under third_party with its MIT license.

## Copy/paste paths

Click JSON path and use Ctrl+A/Ctrl+V, or click **Paste path**. Windows Explorer **Copy as path** quotes are removed when you paste with the button or finish editing (Enter/click outside). **Copy path** and a right-click context menu are also available. Pasted paths over 200 UTF-8 bytes or containing multiple lines are rejected with a message instead of being silently truncated.

## Final audit additions

Interactive file polling/parsing now uses a bounded background worker. Old results are discarded across source changes. `provider.hpp` selects providers through the common interface; `async_source.hpp` implements handoff and cleanup; `log.cpp` records provider failures and recovery without repeating the same error each poll.

A player record may include `bones`: an array of exactly 15 `[x,y]` screen points. Joint order: head, neck, pelvis, left shoulder/elbow/hand, right shoulder/elbow/hand, left hip/knee/foot, right hip/knee/foot. Coordinates are absolute client-area pixels. Omit it if your producer has no authorized bones; the skeleton toggle then draws no skeleton for that record.

For the optional `{ "schema": 1, "entities": [...] }` 3D format, each entity supplies integer `id`, integer `team`, `name`, `health`, `visible`, and `position: [x,y,z]` in meters. Optional `shield` is 0–100; optional `bones` contains the same 15 joints as `[x,y,z]` world points. The camera is fixed at `(0,1.1,0)`, looks along +Z and has a 70-degree vertical FOV. Missing bones are never synthesized. Bounds without bones use a documented 1.8 m display height, not discovered anatomy.
