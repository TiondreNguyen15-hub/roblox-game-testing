# Blockline — City map update

An original Roblox urban shooter starter with a **saved, editable 256 × 196 stud city**. The map is visible in Studio before you press Play.

## Download and play

1. Download [Blockline.rbxlx](https://github.com/TiondreNguyen15-hub/roblox-game-testing/raw/refs/heads/main/Blockline.rbxlx).
2. In Roblox Studio choose **File → Open from File** and select the new download.
3. You should see the whole city immediately. Press **Play** and wait for the countdown.
4. Use **Test → Server & Clients** with two clients for team testing.

Download the updated file again if you already have an earlier copy; GitHub updates do not change files on your computer.

## The map

- Eight enterable ground-floor shops with counters and shelves for cover.
- Central boulevard, raised sidewalks, crosswalks and two rear alley routes.
- Loading platforms with stairs, delivery cars, crates and traffic barriers.
- Apartment windows, shop signs, roof details and warm streetlights.
- Two color-coded team bases with spawn cover and inward-facing player positioning.
- 253 anchored map parts, organized under **Workspace → BlocklineCity**.

The map is stored directly in the place. It remains visible even if gameplay scripts fail. All geometry is original and uses built-in Roblox parts.

## Add only the map to an existing project

Download [BlocklineCity.rbxmx](https://github.com/TiondreNguyen15-hub/roblox-game-testing/raw/refs/heads/main/BlocklineCity.rbxmx). Stop play mode, remove any old BlocklineCity, right-click **Workspace** in Explorer, and use **Insert from File**. Keep exactly one model named **BlocklineCity**. The map model includes spawns but no gameplay scripts; for this game's full setup, open the place file instead.

## Gameplay and editing

WASD to move, mouse to look, click to shoot, right click to aim, R to reload, Shift to sprint, Space to jump. Controller and touch action bindings are included. Five-minute team rounds; first to 30 eliminations wins.

Edit the map directly in Workspace. [Server.luau](./Server.luau) and [Client.luau](./Client.luau) match the scripts embedded in the place. See [START-HERE.txt](./START-HERE.txt) for the complete setup and controls.

## Validation

XML, unique object references, anchored parts, spawn properties and embedded script round trips checked. Both scripts pass Lua 5.4 syntax checks using a compatible subset. A geometric standing-player clearance check connects both bases, both rear alleys and all eight shop interiors.

Roblox Studio is unavailable in the build environment, so actual Studio loading, multiplayer, lighting, camera and device behavior still require playtesting. No persistence, custom animations, weapon audio, cosmetics or monetization are included.
