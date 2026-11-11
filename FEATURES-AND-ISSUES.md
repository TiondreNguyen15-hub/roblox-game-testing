# BLOCKLINE — completed features and remaining issues

This build implements the requested systems as an integrated Roblox prototype. It is **not yet verified in Roblox Studio or on physical devices**. Automated checks use API test doubles and geometry/math checks; they do not substitute for the Roblox engine.

| Phase | Implemented | Validation |
|---|---|---|
| 0 — Intro/menu | Original B/ BLOCKLINE branding, charcoal/red ScreenGui, account/asset progress, arena backdrop, transitions, hover/controller focus, PLAY, practice, loadout, saved settings, stats, profile/level/currency, daily rewards, avatar/weapon previews and actual server roster | Menu construction, button wiring, persistence rules, cloud failures and layout bounds tested headlessly |
| 1 — Existing arena | Editable dark rectangular arena, black/red outlined cover, enclosed bounds, sky/clouds, symmetrical spawns and open lanes | Geometry/collision-clearance checks passed; live physics pending |
| 2 — Movement | Shoulder camera, mouse/touch/right-stick aim, camera smoothing and obstruction handling, sprint, jump, toggle crouch, smooth character rotation, sensitivity; stock Roblox locomotion/jump animations plus additive stance/aim poses | Camera math/input and server stance/speed checks passed; live controls, rig collisions and animation appearance pending |
| 3 — Weapons | Three selectable guns, magazine counts, reloads, individual fire rates/damage/range, server spread and aim spread, local recoil, head damage, authoritative raycasts and procedural weapon recoil/reload animation | Timing, ammo, reload/sprint restrictions, nested hit rigs, invalid directions, damage and magazine-switch regression tests passed |
| 4 — Combat | Health, directional damage feedback, red damage flash, kills/deaths, elimination message, death feedback, Roblox respawn, three-second ForceField spawn protection and kill feed | Fake target/damage input rejection, hit rules, duplicate reward rejection and protection rules tested headlessly |
| 5 — Modes | TDM, FFA and two-slot 1v1 duel in separate arenas; current-server lobby selection, eight-second countdown, timer, score limit, automatic ending, team assignment and results screen | Mode isolation, duel capacity, FFA neutral combat, real round-controller countdown/score-limit completion and rewards tested with a cooperative clock |
| 6 — HUD | Top-center mode/score/timer, current K/D, crosshair, ammo, health bar, keyboard/controller/touch weapon selection, settings, mouse sensitivity and low-shadow graphics option | Seven viewport bounds checked; actual rendering/safe areas pending |
| 7 — Training | Private stationary, moving, tracking, flick and reaction drills; server-scored shots/hits/accuracy/eliminations/reaction time, in-drill stats and last-drill display | All drill entries, career-stat isolation, ownership, hidden target rejection and reaction scoring tested; rendered moving targets and delayed appearance pending |
| 8 — Optimization | Anchored map, non-physical neon edges, decorative shadows disabled, cached query parameters, rejected-shot rate limiting, bounded client tracer pool, staggered saves, lightweight roster updates, 10 Hz changed-value HUD updates and lazy extra arenas | 10,000 same-timestamp fire requests produced at most one raycast; 100 visual events reused 32 Parts; zero server tracer Parts; no Roblox FPS measurements available |

## Mode rules

- **Team Deathmatch:** 5 minutes, first team to 30, balanced teams, no friendly fire.
- **Free For All:** 5 minutes, first player to 20; neutral players can damage each other.
- **1v1 Duel:** 3 minutes, first to 5; two slots. A full lobby returns a clear full-lobby message.
- All competitive modes need two players, cancel if fewer remain, and show real round results. Ties are draws.
- Match completion grants 100 XP + 25 credits; victory adds 50 credits. Eliminations grant 50 XP + 20 credits. Abandoned rounds grant no completion reward.
- Practice never grants competitive kills/deaths, XP, credits or wins. Each drill resets session training counters.

## Training behavior

- Stationary: fixed targets with body/head zones and resetting health.
- Moving: faster lateral movement.
- Tracking: slower continuous lateral motion for sustained aim.
- Flick: target locations randomize after eliminations.
- Reaction: targets become hittable after a random server delay; reaction time is measured from server activation to elimination.
- Accuracy counts server-accepted shots and damaging hits. Reaction time includes the time needed to eliminate the target; it is not a client-estimated click latency.

## Remaining issues and release gates

1. **Roblox Studio is unavailable here.** Actual movement physics, camera feel, shooting feel, font rendering, HUD layering, avatar rigs and animation appearance need Studio playtesting. Crouch lowers stance/speed and prevents jumping; it does not replace Roblox's character with a custom reduced-height collision capsule.
2. **60+ FPS is a target, not a measured result.** Test ten-minute sessions at intended player counts on PC, mobile, Xbox and PlayStation. Capture average and p95/p99 frame time, memory, server script/physics cost and network traffic. Runtime shot counters are under ReplicatedStorage/Blockline attributes.
3. **Real multiplayer replication and latency remain unverified.** Mock tests do not emulate latency, streaming, physics ownership or packet ordering. Hit detection uses current server poses without historical lag compensation.
4. **Physical-device compatibility remains unverified.** Native bindings are implemented; test actual touch devices, Xbox and PlayStation controllers and console safe areas. Camera tests use mathematical substitutes and cannot establish rendered camera collision behavior.
5. **Persistence needs a published experience.** Enable Studio API access on a test experience for Studio data tests. Cloud failures are reported and fallback state never overwrites a saved profile. Unsaved progress can be lost during an outage; a failed release can retain the 150-second session lease.
6. **Weapon/stance poses are procedural local presentation.** Default Roblox Animate supplies normal locomotion/jumping; no custom uploaded motion-capture assets are included. Test standard R6/R15 and intended custom avatars. Other clients see normal replicated locomotion and server stance changes; local weapon presentation is not a separately synchronized animation asset.
7. **Lobby selection is within the current server.** Cross-server skill matchmaking and party invitation services are not included. The team section shows real server players and mode/team assignments.
8. **No full movement/aim anti-cheat is claimed.** Damage, ammo, rate limits, ownership and mode rules are server validated; this does not prevent every speed/aim exploit.
9. **Publishing to Roblox remains a separate step.** Download the current GitHub ZIP and open the included place in Studio. Updating GitHub does not automatically update any live Roblox experience.

## Packaged files

- Blockline.rbxlx: full integrated game.
- Arena-Phase1.rbxlx: map-only edition.
- BlocklineArena.rbxmx: insertable map model.
- Server.luau / Client.luau / Movement.luau: exact sources embedded in the place.
- tools/: reproducible builders and headless regression checks.
- STUDIO-TESTS.md: live acceptance checklist.

## Reproduce automated checks

From the extracted project root:

```
python tools/build_game.py
python tools/test_arena.py
python tools/run_lua.py tools/test_optimization.lua
python tools/run_lua.py tools/test_phases.lua
python tools/run_lua.py tools/test_movement.lua
```

Builders require standard Python. The headless Lua runner requires a Linux Lua 5.4 shared library. Build results are written to output/. The root Blockline.rbxlx included in the ZIP is already built.
