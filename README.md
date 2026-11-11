# BLOCKLINE — integrated shooter prototype

**[Download the full game ZIP](https://github.com/TiondreNguyen15-hub/roblox-game-testing/raw/refs/heads/main/Blockline-Full.zip)**

Open the included **Blockline.rbxlx** using **Roblox Studio → File → Open from File**, then press Play. A browser will display XML if you open the place file directly as a web page; extract the ZIP and open the file inside Roblox Studio.

This build includes the dark/red introduction and menu, third-person shoulder camera, sprint/jump/crouch, three guns, combat feedback, TDM/FFA/1v1 duel, results, responsive HUD, persistent progression and five private training drills.

See **[FEATURES-AND-ISSUES.md](FEATURES-AND-ISSUES.md)** for the full feature matrix, automated test results and remaining issues. Roblox Studio and physical devices were unavailable during implementation. **60+ FPS and live multiplayer/device correctness have not been established.**

## Use the menu

- PLAY cycles between Team Deathmatch, Free For All, 1v1 Duel and Aim Training. Select the mode, then press the large PLAY button.
- PRACTICE cycles between Stationary, Moving, Tracking, Flick and Reaction drills. START enters a private range.
- LOADOUT equips owned guns or unlocks others with earned credits.
- SETTINGS saves FOV, sensitivity, reduced motion, crosshair visibility and the low-shadow graphics option.
- STATISTICS shows saved career stats; TEAM shows the actual current-server roster.
- Daily rewards grant 100 credits once per 24 hours, verified and saved by the server.

## Data saving

Publish a test copy. For Studio persistence tests, enable **Game Settings → Security → Enable Studio Access to API Services** for that test experience. Published servers use Roblox DataStoreService. If access fails, the UI clearly reports session-only mode and blocks cloud purchases/daily claims.

## Controls

| Action | PC | Xbox / PlayStation | Mobile |
|---|---|---|---|
| Move / jump | WASD / Space | Standard Roblox controls | Native movement/jump |
| Aim camera | Mouse | Right stick | Drag on right side |
| Fire / aim | Left / right click | RT/LT or R2/L2 | FIRE / AIM |
| Reload | R | X / Square | RELOAD |
| Sprint | Shift | L3 | RUN |
| Toggle crouch | C or Ctrl | R3 | CROUCH |
| Select weapon | 1 / 2 / 3 | R1 cycles owned weapons | WEAPONS opens loadout |
| Menu / resume | M or MENU | Y / Triangle | MENU |
| Menu select / back | Click | A/B or Cross/Circle | Tap |

## Source organization

- ServerScriptService / BlocklineServer: profiles, saving, movement speed/stance validation, weapons, combat, modes, training and rewards.
- StarterPlayerScripts / BlocklineMenu: native ScreenGui, menu/HUD, previews and feedback.
- StarterPlayerScripts / BlocklineMovement: shoulder camera, aiming, control integration, crouch input, stance and procedural weapon presentation.
- ReplicatedStorage / Blockline: server-created request/combat/event remotes and diagnostics.
- Workspace / BlocklineArena: saved map. Additional mode maps are cloned only when first selected; each player's training range is created on entry and removed on exit.

The three root .luau files match the scripts embedded in the place. `tools/build_game.py` regenerates the integrated place and a separate map-only edition. The original city model is a legacy artifact, not the active arena.

## Delivery status

[Download Blockline-Full.zip](https://github.com/TiondreNguyen15-hub/roblox-game-testing/raw/refs/heads/main/Blockline-Full.zip), extract it, then open Blockline.rbxlx in Roblox Studio. This archive includes the integrated game, source, builders and test report.
