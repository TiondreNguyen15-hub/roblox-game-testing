# Roblox Studio acceptance tests

Status: **pending a live Roblox Studio playtest**. Studio is unavailable in the build environment.

## Map-only movement and collision

1. Open Arena-Phase1.rbxlx. Confirm the floor, boundaries and all outlined obstacles are visible before Play.
2. Press Play. Walk from each spawn around its tall shield block into all three shooting lanes.
3. Traverse the perimeter and central crossings. Confirm there are no floor gaps, stuck corners or inaccessible spawns.
4. Walk into all six types of cover. Confirm black blocks collide and red outlines do not snag movement.
5. Jump against the boundaries and tall blocks. Confirm normal movement cannot leave the arena. Test low blocks as deliberate jumpable cover.
6. Confirm the sky is blue, clouds render and neon outlines are visible without showing through occluding blocks.

## Intro and menu

1. Open Blockline.rbxlx. Press Play. Confirm account loading, the loading bar, arena background, original logo and transition into the menu.
2. Confirm the username, current character preview, level, credits and equipped weapon are loaded.
3. Visit every navigation page. Verify mouse hover, controller focus, scrolling and button activation.
4. Resize the window and use Studio's device emulator in phone portrait, phone landscape, tablet and desktop sizes. Confirm readable text, scrollable pages, accessible buttons and no clipped controls.
5. Test reduced motion: camera orbit and page transitions should stop when enabled. Check FOV, mouse sensitivity and crosshair during gameplay.

## Live server, storage and modes

1. Publish a test experience. Enable Studio API access for the test experience, then start two Studio clients using Server & Clients.
2. Enter the arena on both clients. Confirm balanced teams, countdown, spawn positions, movement, shooting, ammo/reload and friendly-fire rejection.
3. Enter practice on one client. Confirm its targets take damage, reset and cannot be hit by another player's private range. Check no practice kill rewards are awarded.
4. Use MENU/M/Y/Triangle to open the menu and resume. Leave the mode. Confirm returning to lobby removes the weapon and does not leave extra ranges behind.
5. Claim the daily reward once. Confirm exactly +100 credits and that a second claim is rejected. Rejoin and confirm the cooldown and balance persisted.
6. Change and save all settings. Leave, wait for the server to release your profile, and rejoin. Confirm the same values load.
7. Earn enough arena credits to unlock VX-9 or DMR-7. Confirm cost is deducted once, ownership persists and equip changes the actual weapon and fire rate.
8. Complete a match. Confirm kills, deaths, wins, XP, level and credits update and survive rejoining. Confirm abandoning a match grants no completion reward.
9. Disable Studio API access in the test experience. Rejoin: confirm the session-only notice, clear settings-save warning, and rejected cloud purchases/daily claims. Confirm previously saved cloud data was not overwritten.

## Physical devices

Use a real touch device and real console/controller where available. Device emulation and mocked input do not establish console compatibility.

- Xbox: A select, B back, Y menu, RT fire, LT aim, X reload, L3 sprint.
- PlayStation: Cross select, Circle back, Triangle menu, R2 fire, L2 aim, Square reload, L3 sprint.
- Mobile: touch navigation, native movement, and FIRE/AIM/RELOAD/RUN/MENU controls.

Record failures from Studio's Output panel with the action that triggered them before marking these tests passed.

## Additional full-build acceptance tests

- Enter all three competitive modes with two clients. Check separate map positions, countdown, timer, score limits, mode isolation, results, respawn protection and draw/abandon behavior. A third player must receive a full-lobby response when joining an occupied duel.
- Verify TDM team balancing each round. Verify neutral FFA/duel players can damage opponents while TDM teammates cannot. Check results, kills/deaths and rewards survive rejoining.
- Check third-person camera mouse, touch and right-stick input; camera obstruction at every cover type; aiming from behind cover; recoil/spread; sprint/jump/crouch and stance restoration after mode changes or death.
- Test stock R6/R15 locomotion/jumping, additive crouch/aim poses and procedural reload/recoil. Confirm custom avatars do not clip unexpectedly.
- Run all five training drills. Check movement/reset behavior, hidden reaction targets, random flick locations, accepted shot/hit accuracy, reaction-to-elimination timing and career-stat isolation.
- Confirm top-center timer/score, current K/D, kill feed, directional damage, elimination/death text, weapon selection, health bar, training stats and results panels at all supported resolutions.
- Confirm controls pause in the menu and resume in gameplay; test default PlayerModule availability and custom control integrations if used.
- Profile simultaneous active modes and private ranges at intended player counts, including map clones, sustained SMG fire, server heartbeats, pooled tracers and repeated practice joins/exits. Measure 60+ FPS rather than inferring it from object counts.
