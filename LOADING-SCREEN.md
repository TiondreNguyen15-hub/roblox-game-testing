# Part 1 — cinematic loading screen

Implemented independently from the existing menu. No new menu, social lobby, matchmaking zones, ranked/tournament systems or universal weapon changes were made in this stage. Those later parts remain outstanding.

- ReplicatedFirst boot script displays the loader before server/profile initialization.
- Full-screen charcoal/red composition, original BLOCKLINE wordmark and detailed original 3D rifle showcase, with 30+ anchored non-physical parts in a ViewportFrame.
- Rotating gameplay tips, subtle showcase movement and reduced-motion support.
- Status messages for connecting, loading profile, preparing assets, failures, timeout and completion.
- Animated progress uses actual completed ContentProvider callbacks and total deduplicated character/arena/sky asset requests. No time-based fake loading percentage. A 30-second timeout transitions with an explicit unavailable-assets warning, without forcing an incomplete count to 100%.
- Background music with live volume up/down and mute/restore; keyboard/mouse, touch and controller-selectable buttons.
- The configured music asset is rbxassetid://1843529606. Playback requires Roblox permission for this experience. If it is denied or fails, the screen explicitly says music is unavailable. Replace MUSIC_ID in LoadingScreen.luau with an authorized asset if needed. Playback and licensing/permissions have not been verified in this environment.
- Smooth group fade and music fade before the existing menu. Music, UI and render connection are cleaned up on completion.

## Scripts

- ReplicatedFirst/BlocklineLoading — LoadingScreen.luau, presentation and audio controls.
- ReplicatedFirst/BlocklineLoadingBoot — LoadingBoot.luau, early startup and default-screen removal.
- StarterPlayerScripts/BlocklineMenu — Client.luau, supplies real profile/asset completion status and hands off to the existing menu.

## Tests

Headless checks passed: singleton startup/finish, showcase construction, 2/5 assets displaying 40%, unavailable-asset messaging, status updates, mute/volume restore/clamping, rotating tips and reduced-motion code path. Existing game/menu/weapon regression tests and unpublished-place startup test passed.

Roblox Studio is unavailable here. Live visual quality, true Roblox asset callbacks, audio permission/playback, controller focus, mobile safe areas, fade appearance and cleanup must still be verified in Studio. This screen is implemented, but live acceptance testing is not complete. Open Blockline.rbxlx and press Play; do not use the map-only place.

Run: python tools/run_lua.py tools/test_loading.lua
